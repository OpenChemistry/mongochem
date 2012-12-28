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

#ifndef OBJECTREF_H
#define OBJECTREF_H

#include <mongo/client/dbclient.h>

namespace MongoChem {

/** \brief The ObjectRef class represents an abstract object in a database. */
class ObjectRef
{
public:
  /** Creates a new object reference with \p id_. */
  ObjectRef(const mongo::OID &id_ = mongo::OID()) : m_id(id_) { }

  /** Destroys the object reference. */
  virtual ~ObjectRef() { }

  /** Returns the id of the referenced object. */
  mongo::OID id() const { return m_id; }

  /** Returns \c true if the object reference is valid. */
  bool isValid() const { return m_id.isSet(); }

private:
  mongo::OID m_id;
};

} // end MongoChem namespace

#endif // OBJECTREF_H
