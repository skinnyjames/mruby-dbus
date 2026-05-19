
notification_args = ["mruby-app", 0, "dialog-information", "Hello", "fuck yeah!", [], {"image-path" => ["s", "/home/skinnyjames/smile.png"], "x" => ["(sai)", ["foo", [1,2,3]]]}, 0]
$notify = SDBus.user.service("org.freedesktop.Notifications").object("/org/freedesktop/Notifications").interface("org.freedesktop.Notifications")
$notify.call("Notify", notification_args)

iface = SDBus.user.service("org.mpris.MediaPlayer2.firefox.instance_1_91").object("/org/mpris/MediaPlayer2").interface("org.mpris.MediaPlayer2.Player")
iface.on("Seeked") do |payload|
  secs = payload / 1_000_000
  notification_args = [
    "Yo boy sean",
    0,
    "dialog-information",
    "Playhead changed",
    secs.to_s,
    [],
    {"image-path" => ["s", "/home/skinnyjames/smile.png"], "x" => ["(sai)", ["foo", [1,2,3]]]},
    0
  ]
  $notify.call("Notify", notification_args)
end


while true
  usleep 10000

  SDBus.user.next
end