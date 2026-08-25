// yaini - Yet another INI parser
// Copyright (C) 2010-2022 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "schema.hpp"

#include <sstream>

namespace yaini {

class PairSchemaBoolCallback : public PairSchema
{
private:
  std::function<void ()> m_true_callback;
  std::function<void ()> m_false_callback;

public:
  PairSchemaBoolCallback(std::function<void ()> true_callback,
                            std::function<void ()> false_callback) :
    m_true_callback(true_callback),
    m_false_callback(false_callback)
  {}

  void call(const std::string& value) override
  {
    bool v = false;
    if (value == "yes" || value == "true" || value == "1")
    {
      v = true;
    }
    else if (value == "no" || value == "false" || value == "0")
    {
      v = false;
    }
    else
    {
      throw std::runtime_error("unable to convert '" + value + "' to bool");
    }

    if (v)
    {
      if (m_true_callback) m_true_callback();
    }
    else
    {
      if (m_false_callback) m_false_callback();
    }
  }

  std::string str() const override
  {
    // FIXME: implement me
    return "<not implemented>";
  }
};

class PairSchemaBool : public PairSchema
{
private:
  bool* m_data;

public:
  PairSchemaBool(bool* data) : m_data(data) {}
  void call(const std::string& value) override
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

  std::string str() const override
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

private:
  PairSchemaBool(const PairSchemaBool&);
  PairSchemaBool& operator=(const PairSchemaBool&);
};

class PairSchemaInt : public PairSchema
{
private:
  int* m_data;

public:
  PairSchemaInt(int* data) : m_data(data) {}
  void call(const std::string& value) override
  {
    *m_data = atoi(value.c_str());
  }

  std::string str() const override
  {
    std::ostringstream out;
    out << *m_data;
    return out.str();
  }

private:
  PairSchemaInt(const PairSchemaInt&);
  PairSchemaInt& operator=(const PairSchemaInt&);
};

class PairSchemaFloat : public PairSchema
{
private:
  float* m_data;

public:
  PairSchemaFloat(float* data) : m_data(data) {}
  void call(const std::string& value) override
  {
    *m_data = static_cast<float>(atof(value.c_str()));
  }

  std::string str() const override
  {
    std::ostringstream out;
    out << *m_data;
    return out.str();
  }

private:
  PairSchemaFloat(const PairSchemaFloat&);
  PairSchemaFloat& operator=(const PairSchemaFloat&);
};

class PairSchemaString : public PairSchema
{
private:
  std::string* m_data;

public:
  PairSchemaString(std::string* data) : m_data(data) {}
  void call(const std::string& value) override
  {
    *m_data = value;
  }

  std::string str() const override
  {
    // FIXME: implement proper escaping
    return *m_data;
  }

private:
  PairSchemaString(const PairSchemaString&);
  PairSchemaString& operator=(const PairSchemaString&);
};

class PairSchemaCallback : public PairSchema
{
private:
  std::function<void (const std::string&)> m_callback;

public:
  PairSchemaCallback(std::function<void (const std::string&)> callback) :
    m_callback(callback)
  {}

  void call(const std::string& value) override
  {
    if (m_callback)
      m_callback(value);
  }

  std::string str() const override
  {
    // FIXME: implement me
    return "<not implemented>";
  }
};

SchemaSection::SchemaSection(std::function<void (const std::string&, const std::string&)> callback) :
  m_schema(),
  m_callback(callback)
{
}

SchemaSection::~SchemaSection()
{
}

SchemaSection&
SchemaSection::add(const std::string& name, std::unique_ptr<PairSchema> schema)
{
  m_schema[name] = std::move(schema);

  return *this;
}

SchemaSection&
SchemaSection::operator()(const std::string& name, bool*  value)
{
  add(name, std::make_unique<PairSchemaBool>(value));
  return *this;
}

SchemaSection&
SchemaSection::operator()(const std::string& name, int*   value)
{
  add(name, std::make_unique<PairSchemaInt>(value));
  return *this;
}

SchemaSection&
SchemaSection::operator()(const std::string& name, float* value)
{
  add(name, std::make_unique<PairSchemaFloat>(value));
  return *this;
}

SchemaSection&
SchemaSection::operator()(const std::string& name, std::string* value)
{
  add(name, std::make_unique<PairSchemaString>(value));
  return *this;
}

SchemaSection&
SchemaSection::operator()(const std::string& name,
                             std::function<void ()> true_callback,
                             std::function<void ()> false_callback)
{
  add(name, std::make_unique<PairSchemaBoolCallback>(true_callback, false_callback));
  return *this;
}

SchemaSection&
SchemaSection::operator()(const std::string& name, std::function<void (const std::string&)> callback)
{
  add(name, std::make_unique<PairSchemaCallback>(callback));
  return *this;
}

PairSchema*
SchemaSection::get(const std::string& name) const
{
  auto i = m_schema.find(name);
  if (i == m_schema.end())
  {
    return nullptr;
  }
  else
  {
    return i->second.get();
  }
}

void
SchemaSection::save(std::ostream& out)
{
  for (auto i = m_schema.begin(); i != m_schema.end(); ++i)
  {
    out << i->first << " = " << i->second->str() << std::endl;
  }
}

Schema::Schema() :
  m_sections()
{
}

Schema::~Schema()
{
  clear();
}

void
Schema::clear()
{
  m_sections.clear();
}

SchemaSection&
Schema::section(const std::string& name,
                   std::function<void (const std::string&, const std::string&)> callback)
{
  auto sec = std::make_unique<SchemaSection>(callback);
  auto* const sec_tmp = sec.get();
  m_sections[name] = std::move(sec);

  return *sec_tmp;
}

SchemaSection*
Schema::get_section(const std::string& name) const
{
  auto const i = m_sections.find(name);
  if (i != m_sections.end())
  {
    return i->second.get();
  }
  else
  {
    return nullptr;
  }
}

void
Schema::save(std::ostream& out)
{
  for (auto i = m_sections.begin(); i != m_sections.end(); ++i)
  {
    out << "[" << i->first << "]" << std::endl;
    i->second->save(out);
    out << std::endl;
  }
  out << "\n# EOF #" << std::endl;
}

} // namespace yaini

/* EOF */
