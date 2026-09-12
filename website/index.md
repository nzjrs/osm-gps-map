---
layout: default
permalink: /
---

{% if site.data.releases.latest %}
<p>Download <a href="{{ site.data.releases.latest.archive_url }}">{{ site.data.releases.latest.tag }} archive</a>
(<a href="{{ site.data.releases.latest.html_url }}">Release notes for {{ site.data.releases.latest.tag }}</a>).</p>
{% else %}
<p>No GitHub Release yet. Tag-only publishes gtk-doc; Download fills in when a Release exists.</p>
{% endif %}

A Gtk+ widget (and Python bindings) that when given GPS co-ordinates, draws a GPS track, and points of interest on a moving map display. Downloads map data from a number of websites, including openstreetmap.org. The library has excellent performance and is currently used in a number of Gtk+ applications.

![osm-gps-map showing an OpenStreetMap track in Christchurch]({{ '/screenshot.png' | relative_url }})

![osm-gps-map with on-screen display over satellite imagery]({{ '/screenshot-osd.png' | relative_url }})

Currently supports a number of different mapping sources:

- OpenStreetMap (default)
- OpenTopoMap
- OpenCycleMap
- Maps-For-Free
- Google Maps / Satellite and Bing Maps (Virtual Earth)

It also has the following features:

- Intelligent, flexible and customizable caching of maps, including the ability to request a specific area of the map to be cached ahead of time
- Recording of points of interest on the map (and the ability to add arbitrary pixmaps at those points)
- Automatically draws a GPS track (a line showing the history of past added points)
- Automatic centering on new GPS points
- Support for multiple other tracks of co-ordinate points
- Adjustable Zoom
- Built in support for keyboard navigation
- Includes a [set of examples](https://github.com/{{ site.data.releases.repo }}/tree/master/examples/)
- Simple, [flat API](https://github.com/{{ site.data.releases.repo }}/blob/master/src/osm-gps-map-widget.h)
- Support for showing additional display layers rendered on top of the map
- Optional on screen display (OSD)
- Proper map copyright attribution and user-agent usage to comply with various map requirements

## Documentation

- [API Documentation]({{ '/docs/reference/html/index.html' | relative_url }})
- Code examples
  - [mapviewer.c](https://github.com/{{ site.data.releases.repo }}/blob/master/examples/mapviewer.c)
  - [mapviewer.py](https://github.com/{{ site.data.releases.repo }}/blob/master/examples/mapviewer.py)
  - [mapviewer.js](https://github.com/{{ site.data.releases.repo }}/blob/master/examples/mapviewer.js) (JavaScript example using GObject Introspection)
  - [polygon.c](https://github.com/{{ site.data.releases.repo }}/blob/master/examples/polygon.c) (small polygon-on-map drawing example)
  - [editable_track.c](https://github.com/{{ site.data.releases.repo }}/blob/master/examples/editable_track.c) (small example of an editable track feature)
- Please e-mail the [mailing list](https://groups.google.com/group/osm-gps-map) for all questions.
- Report issues using the canonical [issue tracker](https://github.com/nzjrs/osm-gps-map/issues)

## Install Instructions

osm-gps-map should be packaged by your distribution. On Debian, Ubuntu or similar, you can install the GTK+ library and its Python bindings like this:

```
sudo apt install libosmgpsmap-1.0-1 gir1.2-osmgpsmap-1.0 python3-gi
```

And if you want to build some application that depends on osm-gps-map, you can install the development files like this:

```
sudo apt install libosmgpsmap-1.0-dev
```

### Installing From Source

To build from source on Debian or Ubuntu you will need:

```
sudo apt install build-essential autoconf automake libtool \
  libgtk-3-dev libsoup-3.0-dev libgirepository1.0-dev
```

Then:

```
./autogen.sh && make && sudo make install
```

{% if site.data.releases.latest and site.data.releases.latest.changes.size > 0 %}
## News

<p>Changes in {{ site.data.releases.latest.tag }} ({{ site.data.releases.latest.date }})</p>
<ul>
{% for item in site.data.releases.latest.changes %}
  <li>{{ item }}</li>
{% endfor %}
</ul>
{% endif %}

## Download

You may download any of the following archives:

{% if site.data.releases.archives.size > 0 %}
<ul>
{% for rel in site.data.releases.archives %}
  <li><a href="{{ rel.archive_url }}">{{ rel.tag }} archive</a> (<a href="{{ rel.html_url }}">Release notes for {{ rel.tag }}</a>)</li>
{% endfor %}
</ul>
{% else %}
<ul><li>(no GitHub Releases yet)</li></ul>
{% endif %}

You can also clone the project with [Git](https://git-scm.com) by running:

```
$ git clone {{ site.data.releases.clone_url }}
```

A snapshot of master is also available as [tar]({{ site.data.releases.master_tar }}) or [zip]({{ site.data.releases.master_zip }}).

## License

- GPLv2+

## Authors

- John Stowers
- Till Harbaum
- Alberto Mardegan
- Mark Cottrell
- Originally based on tangoGPS by Marcus Bauer

## Contact

- osm-gps-map [mailing list](https://groups.google.com/group/osm-gps-map)
- Canonical [issue tracker](https://github.com/nzjrs/osm-gps-map/issues)

get the source code on GitHub : [{{ site.data.releases.repo }}](https://github.com/{{ site.data.releases.repo }})
