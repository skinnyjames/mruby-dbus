MRuby::Gem::Specification.new('mruby-sdbus') do |spec|
  spec.license = 'MIT'
  spec.author  = 'skinnyjames'
  spec.summary = 'mruby sdbus bindings'
  spec.version = '0.1.0'
  spec.add_dependency 'mruby-xml', github: 'skinnyjames/mruby-xml', branch: 'main'
  spec.add_dependency 'mruby-sleep', github: 'matsumotory/mruby-sleep'
end
