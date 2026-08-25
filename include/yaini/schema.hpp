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

#ifndef HEADER_YAINI_SCHEMA_HPP
#define HEADER_YAINI_SCHEMA_HPP

#include <functional>
#include <map>
#include <memory>
#include <string>

namespace yaini {

class PairSchema
{
public:
  PairSchema() {}
  virtual ~PairSchema() {}
  virtual std::string str() const =0;
  virtual void call(const std::string& value) =0;
};

class SchemaSection
{
private:
  std::map<std::string, std::unique_ptr<PairSchema>> m_schema;

public:
  std::function<void (const std::string&, const std::string&)> m_callback;

public:
  SchemaSection(std::function<void (const std::string&, const std::string&)> callback);
  ~SchemaSection();

  SchemaSection& operator()(const std::string& name, bool*  value);
  SchemaSection& operator()(const std::string& name, int*   value);
  SchemaSection& operator()(const std::string& name, float* value);
  SchemaSection& operator()(const std::string& name, std::string* value);
  SchemaSection& operator()(const std::string& name, std::function<void (const std::string&)> callback);
  SchemaSection& operator()(const std::string& name,
                               std::function<void ()> true_callback,
                               std::function<void ()> false_callback);

  PairSchema* get(const std::string& name) const;

  void save(std::ostream& out);

private:
  SchemaSection& add(const std::string& name, std::unique_ptr<PairSchema> schema);

private:
  SchemaSection(const SchemaSection&);
  SchemaSection& operator=(const SchemaSection&);
};

class Schema
{
private:
  std::map<std::string, std::unique_ptr<SchemaSection>> m_sections;

public:
  Schema();
  ~Schema();

  void clear();

  SchemaSection& section(const std::string& name,
                            std::function<void (const std::string&, const std::string&)> callback
                            = std::function<void (const std::string&, const std::string&)>());

  SchemaSection* get_section(const std::string& name) const;

  void save(std::ostream& out);

private:
  Schema(const Schema&);
  Schema& operator=(const Schema&);
};

} // namespace yaini

#endif

/* EOF */
