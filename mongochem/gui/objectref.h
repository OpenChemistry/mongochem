/******************************************************************************

  This source file is part of the MongoChem project.

  Copyright 2012-2013 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef MONGOCHEM_OBJECTREF_H
#define MONGOCHEM_OBJECTREF_H

#include <string>

namespace MongoChem {

/** \brief The ObjectRef class represents an abstract object in a database. */
class ObjectRef
{
private:
  typedef void (ObjectRef::*boolType)();
  void trueBoolType() { }

public:
  /** Creates a new object reference with \p id. */
  ObjectRef(const std::string &id = "");

  /** Returns the id of the referenced object. */
  std::string id() const { return m_id; }

  /** Returns \c true if the object reference is valid. */
  bool isValid() const;

  /**
   * Returns @c true if the ObjectRef refers to a valid object.
   *
   * This allows the reference to be used as a predicate:
   * @code
     ObjectRef ref = ...
     if (ref)
       // use ref
   * @endcode
   */
  operator boolType() const
  {
    return isValid() ? &ObjectRef::trueBoolType : 0;
  }

private:
  std::string m_id;
};

} // end MongoChem namespace

#endif // MONGOCHEM_OBJECTREF_H
