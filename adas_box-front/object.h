#pragma once

/**
 * base class not allowed copy
 */
class Object {
public:
	Object() {}
private:
	Object(const Object&);
	Object& operator = (const Object&);
};

