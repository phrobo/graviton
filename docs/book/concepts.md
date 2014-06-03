Graviton: The Overview
======================

Graviton aims to solve a highly specific set of problems:

* Exposing user-hosted and networkable services with a remote procedure call
  (RPC) API and generic data streaming API, similar to D-Bus and GVFS.
* Abstracting away all the nitty gritty and often painful bits of networking:
  sockets, DNS resolution, network browsing, port forwarding, living behind
  firewalls, ipv6, 
* Letting developers do all of this in as few lines of code as needed
* Removing all burden of configurability from the user

To accomplish this, things are organized into a few major concepts.


Node and Cloud IDs
------------------

In computer networking, there exists a concept of addresses and ports. An
address is a pointer to another computer on the network, while a port refers to
the specific endpoint on that computer that a service is listening on. When
another device wants to connect to a remote service, it needs to know both the
port and address before any kind of connectivity can start.

In Graviton, you only need to worry about Node IDs and Cloud IDs. Even then,
their usage is very infrequent.

Both IDs are glorified GUIDs. Cloud IDs indicate a specific collection of nodes,
while a Node ID points to a specific node within the cloud. Neither ID contains
any inherit routing or connectivity information such as IP addresses, subnets,
or even TCP ports. To connect to a node, one merely asks Graviton to do so and
it'll figure it out. This means that the real-world connection might happen via
JSON-RPC on HTTP, local D-Bus access, or even rudimentary interprocess IPC using
shared memory. Developers do not need to know about how it works, just that it
does.

Clouds and Nodes
----------------

The largest organizational structure in Graviton is a Cloud. Clouds are
collections of nodes that provide services. There is no defined route "through"
a cloud to get to a specific service or node, except at the implementation
level. Services are found within a cloud by asking the cloud as a whole to
locate any such services.

Each cloud is composed of zero or more Nodes. A node is a single endpoint within
the cloud that provides a number of services. Multiple nodes may be running on a
single physical machine, but that distinction is abstracted away. You interact
with a node through its exposed services.

Services and Interfaces
-----------------------

Each node hosts a number of services. These may be things such as an entry point
to a file storage system, an internet radio source, or an interface to
lock/unlock a physical door with some real-world hardware. Services are
identified by simple namespaced strings, such as the introspection service:
net:phrobo:graviton:introspection. Services define the set of methods,
properties, streams, etc that are available on a given node.

The client side of a service is called a Service Interface. Clever, huh?

Methods, Properties, and Streams
--------------------------------

Each Service has a set of methods, properties, and streams. Methods are
operations that can be invoked on a service with optional input (aka arguments)
and optionally produce output (aka return value). For example, a jukebox service
might provide the methods play(someUrl), stop(), and pause().

Properties are simple attributes of a service. The above jukebox service might
provide a currentTrack property and a isPlaying property.

Streams are arbitrary data streams. They have all the semantics of a regular
file in that you can read, write, seek, and perform a few out of band operations
(ala ioctl()) on them. However, they are not files. Whatever is on the other end
of the stream could be a random number generator, the tail end of a multimedia
encoding pipeline, an IRC session, or in fact, a real file on disk.

The behavior of a service's Methods, Properties, and Streams are defined
according to whatever interface is being implemented. One should not open a
stream named "random-data" on a jukebox service and expect the same random data
that a random number generator service might provide. It could actually be some
really good chiptunes.

