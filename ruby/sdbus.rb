require_relative "./sdbus/service"

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