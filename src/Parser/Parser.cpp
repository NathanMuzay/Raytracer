/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** Parser
*/

#include "Parser.hpp"

#include <filesystem>

namespace {
	namespace fs = std::filesystem;

	double getRequiredNumber(const libconfig::Setting &setting, const char *name, const std::string &context)
	{
		double value = 0.0;
		int intValue = 0;

		if (setting.lookupValue(name, value))
			return value;
		if (setting.lookupValue(name, intValue))
			return static_cast<double>(intValue);
		throw ParserException("Missing required numeric field '" + std::string(name) + "' in " + context);
	}

	int getRequiredInt(const libconfig::Setting &setting, const char *name, const std::string &context)
	{
		int value = 0;

		if (!setting.lookupValue(name, value))
			throw ParserException("Missing required integer field '" + std::string(name) + "' in " + context);
		return value;
	}

	std::string getRequiredString(const libconfig::Setting &setting, const char *name, const std::string &context)
	{
		std::string value;

		if (!setting.lookupValue(name, value))
			throw ParserException("Missing required string field '" + std::string(name) + "' in " + context);
		return value;
	}

	const libconfig::Setting &getRequiredGroup(const libconfig::Setting &setting, const char *name, const std::string &context)
	{
		if (!setting.exists(name))
			throw ParserException("Missing required section '" + std::string(name) + "' in " + context);
		const libconfig::Setting &group = setting[name];
		if (group.getType() != libconfig::Setting::TypeGroup)
			throw ParserException("Section '" + std::string(name) + "' in " + context + " must be a group");
		return group;
	}

	void validatePosition3(const libconfig::Setting &setting, const std::string &context)
	{
		(void)getRequiredNumber(setting, "x", context);
		(void)getRequiredNumber(setting, "y", context);
		(void)getRequiredNumber(setting, "z", context);
	}

	void validateListType(const libconfig::Setting &parent, const char *name, const std::string &context)
	{
		if (!parent.exists(name))
			return;
		const libconfig::Setting &list = parent[name];
		if (list.getType() != libconfig::Setting::TypeList)
			throw ParserException("Section '" + std::string(name) + "' in " + context + " must be a list");
	}

	void validateBackground(const libconfig::Setting &background)
	{
		if (background.getType() != libconfig::Setting::TypeGroup)
			throw ParserException("'background' must be a group");
		(void)getRequiredNumber(background, "r", "background");
		(void)getRequiredNumber(background, "g", "background");
		(void)getRequiredNumber(background, "b", "background");
	}

	void validateMaterial(const libconfig::Setting &setting, const std::string &context)
	{
		if (!setting.exists("material"))
			return;
		const libconfig::Setting &material = setting["material"];
		if (material.getType() != libconfig::Setting::TypeGroup)
			throw ParserException(context + ".material must be a group");

		std::string type;
		if (!material.lookupValue("type", type) || type.empty())
			throw ParserException(context + ".material must have a non-empty 'type' field");
	}
}

Parser::Parser(const std::string &configFile)
{
	if (!fs::exists(configFile) || !fs::is_regular_file(configFile))
		throw ParserException("Config file not found: " + configFile);
	if (fs::path(configFile).extension() != ".cfg")
		throw ParserException("Config file must have a .cfg extension: " + configFile);
	try {
		_config.readFile(configFile.c_str());
	} catch (const libconfig::FileIOException &) {
		throw ParserException("Cannot read config file: " + configFile);
	} catch (const libconfig::ParseException &e) {
		throw ParserException("Invalid libconfig format in " + configFile + " at line "
			+ std::to_string(e.getLine()) + ": " + e.getError());
	}
	validateConfig();
}

const libconfig::Config &Parser::getConfig() const
{
	return _config;
}

const libconfig::Setting &Parser::getCamera() const
{
	return _config.getRoot()["camera"];
}

const libconfig::Setting &Parser::getPrimitives() const
{
	return _config.getRoot()["primitives"];
}

const libconfig::Setting &Parser::getLights() const
{
	return _config.getRoot()["lights"];
}

void Parser::validateConfig() const
{
	const libconfig::Setting &root = _config.getRoot();

	if (!root.exists("camera"))
		throw ParserException("Missing required top-level section 'camera'");
	if (!root.exists("primitives"))
		throw ParserException("Missing required top-level section 'primitives'");
	if (!root.exists("lights"))
		throw ParserException("Missing required top-level section 'lights'");

	if (root.exists("background"))
		validateBackground(root["background"]);

	validateCamera(root["camera"]);
	validatePrimitives(root["primitives"]);
	validateLights(root["lights"]);
}

void Parser::validateCamera(const libconfig::Setting &camera) const
{
	if (camera.getType() != libconfig::Setting::TypeGroup)
		throw ParserException("'camera' must be a group");

	const libconfig::Setting &resolution = getRequiredGroup(camera, "resolution", "camera");
	const int width = getRequiredInt(resolution, "width", "camera.resolution");
	const int height = getRequiredInt(resolution, "height", "camera.resolution");
	const libconfig::Setting &position = getRequiredGroup(camera, "position", "camera");
	const libconfig::Setting &rotation = getRequiredGroup(camera, "rotation", "camera");
	const double fieldOfView = getRequiredNumber(camera, "fieldOfView", "camera");

	if (width <= 0 || height <= 0)
		throw ParserException("camera.resolution width and height must be positive");
	validatePosition3(position, "camera.position");
	validatePosition3(rotation, "camera.rotation");
	if (fieldOfView <= 0.0 || fieldOfView >= 180.0)
		throw ParserException("camera.fieldOfView must be in ]0, 180[");
}

void Parser::validatePrimitives(const libconfig::Setting &primitives) const
{
	if (primitives.getType() != libconfig::Setting::TypeGroup)
		throw ParserException("'primitives' must be a group");

	validateListType(primitives, "spheres", "primitives");
	validateListType(primitives, "planes", "primitives");

	if (primitives.exists("spheres")) {
		const libconfig::Setting &spheres = primitives["spheres"];

		for (int i = 0; i < spheres.getLength(); ++i) {
			const libconfig::Setting &sphere = spheres[i];
			const std::string context = "primitives.spheres[" + std::to_string(i) + "]";

			if (sphere.getType() != libconfig::Setting::TypeGroup)
				throw ParserException(context + " must be a group");
			const double radius = getRequiredNumber(sphere, "r", context);

			validatePosition3(sphere, context);
			validateMaterial(sphere, context);
			if (radius <= 0.0)
				throw ParserException(context + ".r must be strictly positive");
		}
	}

	if (primitives.exists("planes")) {
		const libconfig::Setting &planes = primitives["planes"];

		for (int i = 0; i < planes.getLength(); ++i) {
			const libconfig::Setting &plane = planes[i];
			const std::string context = "primitives.planes[" + std::to_string(i) + "]";

			if (plane.getType() != libconfig::Setting::TypeGroup)
				throw ParserException(context + " must be a group");
			const std::string axis = getRequiredString(plane, "axis", context);

			(void)getRequiredNumber(plane, "position", context);
			validateMaterial(plane, context);
			if (axis != "X" && axis != "Y" && axis != "Z")
				throw ParserException(context + ".axis must be one of: X, Y, Z");
		}
	}

	if (primitives.exists("objmeshes")) {
		const libconfig::Setting &meshes = primitives["objmeshes"];
		validateListType(primitives, "objmeshes", "primitives");

		for (int i = 0; i < meshes.getLength(); ++i) {
			const libconfig::Setting &mesh = meshes[i];
			const std::string context = "primitives.objmeshes[" + std::to_string(i) + "]";

			if (mesh.getType() != libconfig::Setting::TypeGroup)
				throw ParserException(context + " must be a group");

			std::string meshPath;
			if (!mesh.lookupValue("path", meshPath) || meshPath.empty())
				throw ParserException(context + " must have a non-empty 'path' field");

			validateMaterial(mesh, context);
		}
	}
}

void Parser::validateLights(const libconfig::Setting &lights) const
{
	if (lights.getType() != libconfig::Setting::TypeGroup)
		throw ParserException("'lights' must be a group");

	const double ambient = getRequiredNumber(lights, "ambient", "lights");
	const double diffuse = getRequiredNumber(lights, "diffuse", "lights");

	validateListType(lights, "point", "lights");
	validateListType(lights, "directional", "lights");

	if (ambient < 0.0 || ambient > 1.0)
		throw ParserException("lights.ambient must be between 0 and 1");
	if (diffuse < 0.0 || diffuse > 1.0)
		throw ParserException("lights.diffuse must be between 0 and 1");

	if (lights.exists("point")) {
		const libconfig::Setting &points = lights["point"];

		for (int i = 0; i < points.getLength(); ++i) {
			if (points[i].getType() != libconfig::Setting::TypeGroup)
				throw ParserException("lights.point[" + std::to_string(i) + "] must be a group");
			validatePosition3(points[i], "lights.point[" + std::to_string(i) + "]");
		}
	}
	if (lights.exists("directional")) {
		const libconfig::Setting &directionals = lights["directional"];

		for (int i = 0; i < directionals.getLength(); ++i) {
			if (directionals[i].getType() != libconfig::Setting::TypeGroup)
				throw ParserException("lights.directional[" + std::to_string(i) + "] must be a group");
			validatePosition3(directionals[i], "lights.directional[" + std::to_string(i) + "]");
		}
	}
}
