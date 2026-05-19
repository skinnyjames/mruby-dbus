require_relative "./type_parser"

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