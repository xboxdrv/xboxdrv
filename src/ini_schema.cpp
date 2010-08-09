/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2010 Ingo Ruhnke <grumbel@gmx.de>
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ini_schema.hpp"

#include <sstream>

class INIPairSchema
{
public:
  virtual ~INIPairSchema() {}
  virtual std::string str() const = 0;
};

class INIPairSchemaBool : public INIPairSchema
{
private:
  bool* m_data;
  
public:
  INIPairSchemaBool(bool* data) : m_data(data) {}
  void call(const std::string& value)
  {
    if (value == "yes" || value == "true" || value == "1")
    {
      *m_data = true;
    }
    else if (value == "no" || value == "false" || value == "0")
    {
      *m_data = false;
    }
    else
    {
      throw std::runtime_error("unable to convert '" + value + "' to bool");
    }
  }

  std::string str() const 
  {
    if (m_data)
    {
      return "true";
    }
    else
    {
      return "false";
    }
  }
};

class INIPairSchemaInt : public INIPairSchema
{
private:
  int* m_data;
  
public:
  INIPairSchemaInt(int* data) : m_data(data) {}
  void call(const std::string& value)
  {
    *m_data = atoi(value.c_str());
  }

  std::string str() const 
  {
    std::ostringstream str;
    str << *m_data;
    return str.str();
  }
};

class INIPairSchemaFloat : public INIPairSchema
{
private:
  float* m_data;
  
public:
  INIPairSchemaFloat(float* data) : m_data(data) {}
  void call(const std::string& value)
  {
    *m_data = atoi(value.c_str());
  }

  std::string str() const 
  {
    std::ostringstream str;
    str << *m_data;
    return str.str();
  }
};

class INIPairSchemaString : public INIPairSchema
{
private:
  std::string* m_data;
  
public:
  INIPairSchemaString(std::string* data) : m_data(data) {}
  void call(const std::string& value)
  {
    *m_data = value;
  }

  std::string str() const 
  {
    // FIXME: implement proper escaping
    return *m_data;
  }
};

class INIPairSchemaCallback : public INIPairSchema
{
private:
  boost::function<void (const std::string&)> m_callback;
  
public:
  INIPairSchemaCallback(boost::function<void (const std::string&)> callback) : m_callback(callback) {}
  void call(const std::string& value)
  {
    if (m_callback)
      m_callback(value);
  }

  std::string str() const 
  {
    // FIXME: implement me
    return "<not implemented>";
  }
};

INISchemaSection::INISchemaSection()
{
}

INISchemaSection::~INISchemaSection()
{
  for(Schema::iterator i = m_schema.begin(); i != m_schema.end(); ++i)
  {
    delete i->second;
  }
}

INISchemaSection& 
INISchemaSection::add(const std::string& name, INIPairSchema* schema)
{
  Schema::iterator i = m_schema.find(name);

  if (i != m_schema.end())
  {
    delete i->second;
  }
  
  m_schema.insert(std::pair<std::string, INIPairSchema*>(name, schema));
    
  return *this;
}

INISchemaSection&
INISchemaSection::operator()(const std::string& name, bool*  value)
{
  add(name, new INIPairSchemaBool(value));
  return *this;
}

INISchemaSection& 
INISchemaSection::operator()(const std::string& name, int*   value)
{
  add(name, new INIPairSchemaInt(value));
  return *this;
}

INISchemaSection& 
INISchemaSection::operator()(const std::string& name, float* value)
{
  add(name, new INIPairSchemaFloat(value));
  return *this;
}

INISchemaSection& 
INISchemaSection::operator()(const std::string& name, std::string* value)
{
  add(name, new INIPairSchemaString(value));
  return *this;
}

INISchemaSection& 
INISchemaSection::operator()(const std::string& name, boost::function<void (const std::string&)> callback)
{
  add(name, new INIPairSchemaCallback(callback));
  return *this;
}

void
INISchemaSection::save(std::ostream& out)
{
  for(Schema::iterator i = m_schema.begin(); i != m_schema.end(); ++i)
  {
    out << i->first << " = " << i->second->str() << std::endl;
  }
}

INISchema::INISchema() :
  m_sections()
{
}

INISchema::~INISchema()
{
  clear();
}

void
INISchema::clear()
{
  for(Sections::iterator i = m_sections.begin(); i != m_sections.end(); ++i)
  {
    delete i->second;
  }
  m_sections.clear();
}

INISchemaSection&
INISchema::section(const std::string& name, 
                   boost::function<void (const std::string&, const std::string&)> callback)
{
  Sections::iterator i = m_sections.find(name);
  if (i != m_sections.end())
  {
    delete i->second;
  }

  INISchemaSection* section = new INISchemaSection();
  m_sections.insert(std::pair<std::string, INISchemaSection*>(name, section));
  return *section;
}

INISchemaSection*
INISchema::get_section(const std::string& name)
{
  Sections::iterator i = m_sections.find(name);
  if (i != m_sections.end())
  {
    return i->second;
  }
  else
  {
    return 0;
  }
}

void
INISchema::save(std::ostream& out)
{
  for(Sections::iterator i = m_sections.begin(); i != m_sections.end(); ++i)
  {
    out << "[" << i->first << "]" << std::endl;
    i->second->save(out);
    out << std::endl;
  }
  out << "\n# EOF #" << std::endl;
}

/* EOF */
