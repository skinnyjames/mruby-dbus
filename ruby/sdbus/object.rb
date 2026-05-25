require_relative "./interface"
require_relative "./message"

module SDBus
  class Object
    attr_reader :service, :name

    def initialize(service, name)
      @service = service
      @name = name
    end

    def interfaces
      @interfaces ||= begin
        # fetch introspect xml
        str = service.bus.conn.call(
          service.name,
          name,
          "org.freedesktop.DBus.Introspectable",
          "Introspect",
          "",
          []
        )
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
