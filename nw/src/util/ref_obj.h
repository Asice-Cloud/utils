// Reflectable.h
#ifndef REFLECTABLE_H
#define REFLECTABLE_H

#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

class Reflectable {
public:
	virtual ~Reflectable() = default;

	using FieldGetter = std::function<std::string(const Reflectable*)>;

	struct FieldInfo {
		std::string name;
		FieldGetter getter;
	};

	const std::vector<FieldInfo>& getFields() const {
		return fields;
	}

protected:
	void registerField(const std::string& name, FieldGetter getter) {
		fields.push_back({name, getter});
	}

private:
	std::vector<FieldInfo> fields;
};

#define REFLECTABLE_FIELD(field) \
registerField(#field, [](const Reflectable* obj) -> std::string { \
const auto* derived = static_cast<const std::decay_t<decltype(*this)>*>(obj); \
return std::to_string(derived->field); \
});

#define REFLECTABLE_NESTED_FIELD(field) \
registerField(#field, [](const Reflectable* obj) -> std::string { \
const auto* derived = static_cast<const std::decay_t<decltype(*this)>*>(obj); \
std::string result; \
for (const auto& nestedField : derived->field.getFields()) { \
result += nestedField.name + ": " + nestedField.getter(&derived->field) + "; "; \
} \
return result; \
});

#endif // REFLECTABLE_H