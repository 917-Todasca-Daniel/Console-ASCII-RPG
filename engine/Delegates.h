#ifndef DELEGATES_H
#define DELEGATES_H


template <typename type> class PrimitiveClassMethodDelegate
{
public:
	PrimitiveClassMethodDelegate();
	virtual ~PrimitiveClassMethodDelegate();

	virtual type call() { return type(); }

protected:
private:
};

template <typename type>
PrimitiveClassMethodDelegate <type>::PrimitiveClassMethodDelegate() {

}

template <typename type>
PrimitiveClassMethodDelegate <type>::~PrimitiveClassMethodDelegate() {

}

template <typename type, typename classtype> class ClassMethodDelegate : public PrimitiveClassMethodDelegate <type>
{
public:
	ClassMethodDelegate(classtype* obj, type(classtype::* func)());
	virtual ~ClassMethodDelegate();

	virtual type call() override;

protected:
	type(classtype::* func)();
	classtype *obj;
};

template <typename type, typename classtype>
ClassMethodDelegate <type, classtype>::ClassMethodDelegate(classtype* obj, type(classtype::* func)()) {
	this->obj = obj, this->func = func;
}

template <typename type, typename classtype>
ClassMethodDelegate <type, classtype>::~ClassMethodDelegate() {

}

template <typename type, typename classtype>
type ClassMethodDelegate <type, classtype>::call() {
	if (obj != nullptr && func != nullptr) {
		PrimitiveObject *cast;
		cast = dynamic_cast <PrimitiveObject*> (obj);
		if (cast && cast->wasDestroyed()) return type();
		return (obj->*func)();
	}	return type();
}


template <typename type, typename param> class ParameteredPrimitiveClassMethodDelegate
{
public:
	ParameteredPrimitiveClassMethodDelegate();
	virtual ~ParameteredPrimitiveClassMethodDelegate();

	virtual type call(param value) { return type(); }

protected:
private:
};

template <typename type, typename param>
ParameteredPrimitiveClassMethodDelegate <type, param>::ParameteredPrimitiveClassMethodDelegate() {

}

template <typename type, typename param>
ParameteredPrimitiveClassMethodDelegate <type, param>::~ParameteredPrimitiveClassMethodDelegate() {

}

template <typename type, typename classtype, typename param> class ParameteredClassMethodDelegate : public ParameteredPrimitiveClassMethodDelegate <type, param>
{
public:
	ParameteredClassMethodDelegate(classtype* obj, type(classtype::* func)(param));
	virtual ~ParameteredClassMethodDelegate();

	virtual type call(param value) override;

protected:
	type(classtype::* func)(param);
	classtype* obj;
};

template <typename type, typename classtype, typename param>
ParameteredClassMethodDelegate <type, classtype, param>::ParameteredClassMethodDelegate(classtype* obj, type(classtype::* func)(param)) {
	this->obj = obj, this->func = func;
}

template <typename type, typename classtype, typename param>
ParameteredClassMethodDelegate <type, classtype, param>::~ParameteredClassMethodDelegate() {

}

template <typename type, typename classtype, typename param>
type ParameteredClassMethodDelegate <type, classtype, param>::call(param value) {
	if (obj && func) return (obj->*func)(value);
	return type();
}



template <typename type> class PrimitiveStringClassMethodDelegate
{
public:
	virtual type call(std::string value) = 0;
private:
};

template <typename type, typename classtype> class ParameteredStringClassMethodDelegate : public PrimitiveStringClassMethodDelegate <type>
{
public:
	ParameteredStringClassMethodDelegate(classtype* obj, type(classtype::* func)(std::string));
	virtual ~ParameteredStringClassMethodDelegate();

	virtual type call(std::string value);

protected:
	type(classtype::* func)(std::string);
	classtype* obj;
private:
};

template<typename type, typename classtype>
inline ParameteredStringClassMethodDelegate<type, classtype>::ParameteredStringClassMethodDelegate(classtype* obj, type(classtype::* func)(std::string)) {
	this->obj = obj, this->func = func;
}

template<typename type, typename classtype>
inline ParameteredStringClassMethodDelegate<type, classtype>::~ParameteredStringClassMethodDelegate()
{
}

template<typename type, typename classtype>
inline type ParameteredStringClassMethodDelegate<type, classtype>::call(std::string value) {
	if (obj && func) return (obj->*func)(value);
	return type();
}

#endif // DELEGATES_H
