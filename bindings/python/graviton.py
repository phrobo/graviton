#!/usr/bin/env python
from jsonrpc.proxy import JSONRPCProxy
import subprocess
import json
import requests
import httplib
import dbus, gobject, avahi
from dbus.mainloop.glib import DBusGMainLoop

httplib.HTTPConnection.debuglevel = 1

class Property(object):
  def __init__(self, control, name):
    super(Property, self).__init__()
    self._name = name
    self._control = control 

  @property
  def name(self):
    return self._name

  @property
  def value(self):
    return self._control._node.call(
      "graviton.introspection/getProperty",
      control=self._control.name,
      property=self.name
    )

class Method(object):
  def __init__(self, control, name):
    super(Method, self).__init__()
    self._name = name
    self._control = control

  @property
  def name(self):
    return self._name

class Control(object):
  def __init__(self, node, name):
    super(Control, self).__init__()
    self._name = name
    self._node = node

  @property
  def node(self):
    return self._node

  @property
  def name(self):
    return self._name

  @property
  def properties(self):
    return map(
      lambda x:Property(self, x),
      self._node.call(
        "graviton.introspection/listProperties",
        control=self.name
      )
    )

  @property
  def controls(self):
    return map(
      lambda x:Control(self._node, "%s.%s"%(self._name, x)),
      self._node.call(
        "graviton.introspection/listControls",
        control=self.name
      )
    )

  @property
  def methods(self):
    return map(
      lambda x:Method(self, x),
      self._node.call(
        "graviton.introspection/listMethods",
        control=self.name
      )
    )

  def property(self, name):
    return Property(self, name)

class Node(object):
  def __init__(self, host, port=2718):
    super(Node, self).__init__()
    self._baseurl = "http://%s:%d"%(host, port)
    self._rpc = JSONRPCProxy.from_url(self.url("/rpc"))

  def call(self, *args, **kwargs):
    return self._rpc.call(*args, **kwargs)

  def url(self, suffix=""):
    return "%s/%s"%(self._baseurl, suffix)

  @property
  def events(self):
    return EventWatcher(self)

  def controls(self):
    return map(
      lambda x:Control(self, x),
      self.call("graviton.introspection/listControls")
    )

  def control(self, name):
    return Control(self, name)

  @property
  def name(self):
    return self.control("graviton.introspection").name

class EventWatcher(object):
  def __init__(self, node):
    self._node = node

  def __iter__(self):
    conn = requests.get(self.node.url("events"), stream=True)
    for line in conn.iter_lines(chunk_size=10):
      if line:
        parsed = json.loads(line)
        if 'type' in parsed:
          yield Event(parsed)

  @property
  def node(self):
    return self._node

class Event(object):
  def __init__(self, json):
    assert('type' in json)
    self._data = json

  def __repr__(self):
    return repr(self._data)

  def __str__(self):
    return repr(self._data)

class Browser(object):
  def __init__(self):
    self._pending = 0
    self._gloop = gobject.MainLoop()
    self._loop = DBusGMainLoop()
    self._bus = dbus.SystemBus(mainloop = self._loop)
    self._server = dbus.Interface(
      self._bus.get_object(
        avahi.DBUS_NAME,
        '/'
      ),
      "org.freedesktop.Avahi.Server"
    )
    self._dbrowser = dbus.Interface(
      self._bus.get_object(
        avahi.DBUS_NAME,
        self._server.DomainBrowserNew(
          avahi.IF_UNSPEC,
          avahi.PROTO_UNSPEC,
          "",
          avahi.DOMAIN_BROWSER_BROWSE,
          dbus.UInt32(0)
        )
      ),
      avahi.DBUS_INTERFACE_DOMAIN_BROWSER
    )

    self._started()
    self._dbrowser.connect_to_signal('ItemNew', self._new_domain)
    self._dbrowser.connect_to_signal('AllForNow', self._done)

    self._new_domain(-1, -1, 'local', 32)
    self.results = []

  def _started(self):
    self._pending += 1

  def _done(self, *args):
    self._pending -= 1
    if self._pending == 0:
      self._gloop.quit()

  def _new_domain(self, interface, protocol, domain, flags):
    sbrowser = dbus.Interface(
        self._bus.get_object(
          avahi.DBUS_NAME,
          self._server.ServiceBrowserNew(
            avahi.IF_UNSPEC,
            avahi.PROTO_UNSPEC,
            '_graviton._tcp',
            domain,
            dbus.UInt32(0)
          )
        ),
        avahi.DBUS_INTERFACE_SERVICE_BROWSER
      )
    self._started()
    sbrowser.connect_to_signal('ItemNew', self._handler)
    sbrowser.connect_to_signal('AllForNow', self._done)
    sbrowser.connect_to_signal('Failure', self._done)

  def _handler(self, interface, protocol, name, stype, domain, flags):
    self._started()
    self._server.ResolveService(
      interface,
      protocol,
      name,
      stype,
      domain,
      avahi.PROTO_UNSPEC,
      dbus.UInt32(0),
      reply_handler=self._service_resolved,
      error_handler=self._print_error
    )

  def _print_error(self, *args):
    self._done()
    print args

  def _service_resolved(self, interface, protocol, name, type, domain, host,
      aprotocol, address, port, txt, flags):
    self.results.append(Node(address, port))

  def discover(self):
    self._gloop.run()

  def findByName(self, name):
    self.discover()
    for node in self.results:
      if node.control("graviton").property("hostname").value == name:
        return node
