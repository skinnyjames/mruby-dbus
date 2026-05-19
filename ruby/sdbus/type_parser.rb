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