/******************************************************************************

  This source file is part of the MongoChem project.

  Copyright Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef MONGOCHEM_CHEMKIT_H
#define MONGOCHEM_CHEMKIT_H

#include <boost/shared_ptr.hpp>
#include <vector>

namespace chemkit {
class Molecule;
}

namespace MongoChem {

class MoleculeRef;

/**
 * @class ChemKit
 * @brief The ChemKit class provides utility functions that use the ChemKit
 * libraries to perform certain operations.
 */

class ChemKit
{
public:
  ChemKit();

  /**
   * Creates a new molecule object for @p ref. The ownership of the returned
   * molecule object is passed to the caller.
   */
  static boost::shared_ptr<chemkit::Molecule> createMolecule(
      const MoleculeRef &ref);

  /**
   * Creates a new molecule for @p identifier with @p format. Returns a
   * reference to the newly created molecule.
   *
   * If the molecule already exists in the database no action is performed
   * and a reference to the existing molecule is returned.
   *
   * The following line formats are supported:
   *   - InChI
   *   - SMILES
   *
   * If @p format is not supported a null reference is returned.
   */
  static MoleculeRef importMoleculeFromIdentifier(const std::string &identifier,
                                                  const std::string &format);

  /**
   * @brief Find similar molecules to @p ref in @p refs up to a maximum of @p
   * count.
   * @param ref The reference to the target molecule.
   * @param refs The set of molecules to compare the target molecule to.
   * @param count The maximum number of results to return.
   * @return A vector of similar molecules, in  order to similarity (as
   * determined by the tanimoto similarity value).
   */
  static std::vector<MoleculeRef> similarMolecules(const MoleculeRef &ref,
                                                   const std::vector<MoleculeRef> &refs,
                                                   size_t count);

};

}

#endif // MONGOCHEM_CHEMKIT_H
