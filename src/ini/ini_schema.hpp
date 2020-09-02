/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2010 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_XBOXDRV_INI_SCHEMA_HPP
#define HEADER_XBOXDRV_INI_SCHEMA_HPP

#include <functional>
#include <map>
#include <string>

class INIPairSchema
{
public:
  INIPairSchema() {}
  virtual ~INIPairSchema() {}
  virtual std::string str() const =0;
  virtual void call(const std::string& value) =0;
};

class INISchemaSection
{
private:
  typedef std::map<std::string, INIPairSchema*> Schema;
  Schema m_schema;

public:
  std::function<void (const std::string&, const std::string&)> m_callback;

public:
  INISchemaSection(std::function<void (const std::string&, const std::string&)> callback);
  ~INISchemaSection();

  INISchemaSection& operator()(const std::string& name, bool*  value);
  INISchemaSection& operator()(const std::string& name, int*   value);
  INISchemaSection& operator()(const std::string& name, float* value);
  INISchemaSection& operator()(const std::string& name, std::string* value);
  INISchemaSection& operator()(const std::string& name, std::function<void (const std::string&)> callback);
  INISchemaSection& operator()(const std::string& name,
                               std::function<void ()> true_callback,
                               std::function<void ()> false_callback);

  INIPairSchema* get(const std::string& name) const;

  void save(std::ostream& out);

private:
  INISchemaSection& add(const std::string& name, INIPairSchema* schema);

private:
  INISchemaSection(const INISchemaSection&);
  INISchemaSection& operator=(const INISchemaSection&);
};

class INISchema
{
private:
  typedef std::map<std::string, INISchemaSection*> Sections;
  Sections m_sections;

public:
  INISchema();
  ~INISchema();

  void clear();

  INISchemaSection& section(const std::string& name,
                            std::function<void (const std::string&, const std::string&)> callback
                            = std::function<void (const std::string&, const std::string&)>());

  INISchemaSection* get_section(const std::string& name) const;

  void save(std::ostream& out);

private:
  INISchema(const INISchema&);
  INISchema& operator=(const INISchema&);
};

#endif

/* EOF */
