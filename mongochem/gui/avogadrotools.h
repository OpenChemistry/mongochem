/******************************************************************************

  This source file is part of the MongoChem project.

  Copyright 2013 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef MONGOCHEM_AVOGADROTOOLS_H
#define MONGOCHEM_AVOGADROTOOLS_H

namespace Avogadro {
namespace Core {
class Molecule;
} // end namespace Core
} // end namespace Avogadro

namespace MongoChem {
class MoleculeRef;

/**
 * @class AvogadroTools
 * @brief The AvogadroTools class provides utility functions that use the
 * Avogadro libraries to perform certain operations.
 */
class AvogadroTools
{
public:
  AvogadroTools();

  /**
   * Use chemical json information in @a mcMol to populate @a avoMol. Returns
   * true if a molecule is read.
   */
  static bool createMolecule(const MoleculeRef &mcMol,
                             Avogadro::Core::Molecule &avoMol);
};

} // namespace MongoChem

#endif // MONGOCHEM_AVOGADROTOOLS_H
