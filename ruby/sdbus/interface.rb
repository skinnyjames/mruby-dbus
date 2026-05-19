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
