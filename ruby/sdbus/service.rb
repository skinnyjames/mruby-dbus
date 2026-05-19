require_relative "./object"

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
