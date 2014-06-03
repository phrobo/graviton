An API for the Internet of Things
=================================

The Internet of Things is exactly what it sounds like. You've got these things,
and they're on the internet. Somehow.

Unfortunately, as you may already know, computers are hard. The Internet is
really just a bunch of computers connected together through a patchwork of
devices, protocols, and wishful thinking. For most uses, the Internet performs
exceedingly well:

* Viewing a webpage hosted on a server, accessible by a simple domain name such
  as phrobo.net.
* Streaming multimedia from services like Netflix or Google Music.
* Storing your files on a remote backup service as Dropbox does.
* Checking in on the temperature of your house via Nest.

These uses have had an unimaginable quantity of engineering effort thrown at
them to solve hard problems. Solutions were designed and implemented, but
they're very specific to the application. There is a plethora of other uses that
require a great deal of knowledge about how the internet works and network
communications in general:

* Hosting a peer-to-peer social network on that RaspberryPI server in your
  basement
* Running a shoutcast server to stream your CC-licensed electro-swing chiptune
  collection
* Setting up VPN/SSH/FTP services for remote backups of the cat pictures you've
  collected in your phone
* Getting notifications on your phone when an Amazon delivery activates the DIY
  pressure plate on your front porch

There are many difficult problems involved in a reliable implementation of any
of the above services:

* Your ISP might not give you a static IP address, so you'll need to set up a
  dynamic DNS provider.
* Port forwarding needs configured if you, like others, can't figure out IPV6.
* Maybe the internal network gives DHCP addresses and configuring static
  addresses for port forwarding is out of your control.
* Computers are just plain hard.

Getting these things on the internet might seem like an insurmountable task to
the inexperienced. Even the professionals get frustrated when things stop
working and need reconfigured.

Lets not even think about what you'd need to do if you wanted to connect these
things at home to a friend's things on the other side of the planet.

Graviton solves all of this. It provides a dead simple API for constructing personal
clouds, exposing services on those clouds, and connecting to those services.

Think of it as a zero-configuration toolkit for personal cloud services.
