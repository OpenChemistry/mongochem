/******************************************************************************

  This source file is part of the MongoChem project.

  Copyright 2012 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef MONGOCHEM_MOLECULEREF_H
#define MONGOCHEM_MOLECULEREF_H

#include "objectref.h"

namespace MongoChem {

/** \brief The MoleculeRef class represents a molecule in a database. */
class MoleculeRef : public ObjectRef
{
public:
  /** Creates a new molecule reference with \p str. */
  MoleculeRef(const std::string &str = "") : ObjectRef(str) { }
};

} // end MongoChem namespace

#endif // MONGOCHEM_MOLECULEREF_H
