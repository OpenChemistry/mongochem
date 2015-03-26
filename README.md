MongoChem
=========
![MongoChem][MongoChemLogo]

Introduction
------------

MongoChem is an open-source, cross-platform, desktop application for managing
large collections of chemical data. It uses MongoDB to store and retrieve
data, and can be used in groups to share and search across work being done in
a group. The application uses the Avogadro 2 libraries for interactive 3D
visualization, VTK charts to visualize numberic descriptors and 2D depiction
from Open Babel to show structures in table and chart views. Some highlights:

* Open source distributed under the liberal 3-clause BSD license
* Cross platform with nightly builds on Linux, Mac OS X and Windows
* Multiple chart types (histogram, scatter plot, parallel coordinate)
* Linked selections across charts and table views
* JSON-RPC 2.0 based API for local access from other applications
* Import chemical data from CSV and SDF files
* Plugin architecture enabling extension of the application

![Open Chemistry project][OpenChemistryLogo]
![Kitware, Inc.][KitwareLogo]

MongoChem is being developed as part of the [Open Chemistry][OpenChemistry]
project at [Kitware][Kitware], along with companion tools and libraries to
support the work.

Installing
----------

We provide nightly binaries built by our [dashboards][Dashboard] for Mac OS
X and Windows. If you would like to build from source we recommend that you
follow our [building Open Chemistry][Build] guide that will take care of
building most dependencies.

Contributing
------------

Our project uses the standard GitHub pull request process for code review
and integration. Please check our [development][Development] guide for more
details on developing and contributing to the project. The GitHub issue
tracker can be used to report bugs, make feature requests, etc.

Our [wiki][Wiki] is used to document features, flesh out designs and host other
documentation. Our API is [documented using Doxygen][Doxygen] with updated
documentation generated nightly. We have several [mailing lists][MailingLists]
to coordinate development and to provide support.

  [MongoChemLogo]: http://openchemistry.org/files/logos/mongochem.png "MongoChem"
  [OpenChemistry]: http://openchemistry.org/ "Open Chemistry Project"
  [OpenChemistryLogo]: http://openchemistry.org/files/logos/openchem128.png "Open Chemistry"
  [Kitware]: http://kitware.com/ "Kitware, Inc."
  [KitwareLogo]: http://www.kitware.com/img/small_logo_over.png "Kitware"
  [Dashboard]: http://cdash.openchemistry.org/index.php?project=MongoChem "MongoChem Dashboard"
  [Build]: http://wiki.openchemistry.org/Build "Building MongoChem"
  [Development]: http://wiki.openchemistry.org/Development "Development guide"
  [Wiki]: http://wiki.openchemistry.org/ "Open Chemistry wiki"
  [Doxygen]: http://doc.openchemistry.org/mongochem/api/ "API documentation"
  [MailingLists]: http://openchemistry.org/mailing-lists "Mailing Lists"
