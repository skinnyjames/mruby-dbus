module SDBus
  class Interface
    attr_reader :object, :name, :meta

    def initialize(object, name, meta)
      @object = object
      @name = name
      @meta = meta
      @signals = {}
    end

    def methods
      @meta[:methods]
    end

    def props
      @meta[:props]
    end

    def signals
      @meta[:signals]
    end

    def get(prop_name)
      if prop = props[prop_name]
        raise "Cannot get a writable prop" unless prop[:access] == "read" || prop[:access] == "readwrite"

        conn.get(
          object.service.name,
          object.name,
          name,
          prop_name,
          prop[:type]
        )
      end
    end

    def call(method_name, args = nil)
      if method = methods[method_name]
        argtypes = method[:args].reduce("") do |memo, arg|
          if arg[:direction] == "in"
            memo += arg[:type]
          end

          memo
        end

        conn.call(
          object.service.name,
          object.name,
          name,
          method_name,
          argtypes,
          args
        )
      end
    end

    def on(signal_name, &block)
      if signal = signals[signal_name]
        conn.on(
          object.service.name,
          object.name,
          name,
          signal_name,
          block
        )
      end
    end

    private 

    def type_parser
      @type_parser ||= TypeParser.new
    end

    def conn
      object.service.bus.conn
    end
  end
end

module SDBus
  # unused.
  class TypeParser
    def parse(text)
      parse_chars(text.chars)
    end

    private

    def parse_chars(chars)
      results = []

      while chars.any?
        chars.shift and break if chars.first == ')'
        results.push(parse_one(chars))
      end
      results
    end

    def parse_one(chars)
      case (ch = chars.shift)
      when *%w{y b n q i u x t d h s o g}
        { type: :simple, ident: ch }
      when '('
        children = parse_chars(chars)
        contains = children.map{ |c| c[:ident] }.join('')
        { type: :struct, values: children, contains: contains, ident: "(#{contains})" }
      when 'a'
        if chars.first == '{'
          chars.shift # opening brace
          key   = parse_one(chars)
          value = parse_one(chars)
          chars.shift # closing brace
          contains = "#{key[:ident]}#{value[:ident]}"
          { type: :dict, key: key, value: value, contains: contains, ident: "a{#{contains}}"}
        else
          child = parse_one(chars)
          contains = child[:ident]
          { type: :array, value: child, contains: contains, ident: "a#{contains}" }
        end
      when 'v'
        { type: :variant, ident: 'v' }
      end
    end
  end
end

# notification_args = ["mruby-app", 0, "dialog-information", "Hello", "fuck yeah!", [], {"image-path" => ["s", "/home/skinnyjames/smile.png"], "x" => ["(sai)", ["foo", [1,2,3]]]}, 0]
# SDBus.user.service("org.freedesktop.Notifications").object("/org/freedesktop/Notifications").interface("org.freedesktop.Notifications").call("Notify", notification_args)

module SDBus
  class Message
    attr_accessor :name, :typestr, :params

    def initialize(name, typestr, params)
      @name = name
      @typestr = typestr
      @params = params
    end

    def types
      @types ||= TypeParser.new.parse_and_transform(typestr)
    end
  end
end

module SDBus
  class Object
    attr_reader :service, :name

    def initialize(service, name)
      @service = service
      @name = name
    end

    def interfaces
      @interfaces ||= begin
        p "introspecting"
        # fetch introspect xml
        str = service.bus.conn.introspect(service.name, name)
        xml = XML.parse str
        parse_interfaces(xml)
      end
    end

    def interface(name)
      raise "No interface #{name}" unless interfaces[name]

      Interface.new(self, name, interfaces[name])
    end

    private

    def parse_interfaces(xml)
      value = {}
      interface = xml.child("interface")

      while interface
        iname = interface.attr("name")
        value[iname] = {
          props: {},
          methods: {},
          signals: {},
        }
        # grab properties
        prop = interface.child("property")
        while prop
          type = prop.attr("type")
          name = prop.attr("name")
          access = prop.attr("access")

          value[iname][:props][name] = { type: type, access: access }
          prop = prop.next
        end

        # grab methods
        method = interface.child("method")
        while method
          method_name = method.attr("name")

          args = []
          # method args
          arg = method.child("arg")
          while arg
            type = arg.attr("type")
            name = arg.attr("name")
            direction = arg.attr("direction")

            args << { type: type, name: name, direction: direction }

            arg = arg.next
          end

          value[iname][:methods][method_name] = { args: args }

          method = method.next
        end

        signal = interface.child("signal")
        while signal
          signal_name = signal.attr("name")
          # signal args
          arg = signal.child("arg")
          args = []
          while arg
            type = arg.attr("type")
            name = arg.attr("name")
            args << { type: type, name: name }

            arg = arg.next
          end

          value[iname][:signals][signal_name] = { args: args }
          signal = signal.next
        end

        interface = interface.next
      end

      value
    end
  end
end


module SDBus
  class Service
    attr_accessor :bus, :name

    def initialize(bus, name)
      @bus = bus
      @name = name
    end

    def object(name)
      Object.new(self, name)
    end
  end
end


module SDBus
  class Error < StandardError; end

  def self.system
    Bus.system
  end

  def self.user
    Bus.user
  end

  class Bus
    def self.system
      @system ||= new(SDBus::Connection.system_bus)
    end

    def self.user
      @user ||= new(SDBus::Connection.user_bus)
    end

    def next
      conn.next
    end
    
    attr_reader :conn

    def initialize(conn)
      @conn = conn
    end

    def service(name)
      Service.new(self, name)
    end
  end
end