#pragma once

#include <type_trait.h>
#include <utils.h>

#include <functional>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace reflpp {

class Value;
class Directory;

template <typename T>
inline constexpr bool IsValue = std::is_same_v<RemoveCVRef<T>, Value>;

class List {
   public:
    List() = default;

    List(List&&) = default;
    List(const List&) = default;
    List& operator=(List&&) = default;
    List& operator=(const List&) = default;

   public:
    using iterator = std::vector<Value>::iterator;
    using const_iterator = std::vector<Value>::const_iterator;

    iterator begin() { return list_.begin(); }
    const_iterator begin() const { return list_.begin(); }

    iterator end() { return list_.end(); }
    const_iterator end() const { return list_.end(); }

    const_iterator cbegin() const { return list_.cbegin(); }
    const_iterator cend() const { return list_.cend(); }

    inline void Iteate(std::function<bool(Value&)> handler);
    inline void Iteate(std::function<bool(const Value&)> handler) const;

   public:
    Value& operator[](std::size_t idx) {
        REFLPP_ASSERT(idx < Size());
        return list_[idx];
    }
    const Value& operator[](std::size_t idx) const {
        REFLPP_ASSERT(idx < Size());
        return list_[idx];
    }

    operator bool() const { return !list_.empty(); }

   public:
    void Append(bool v) { AppendBase(v); }
    void Append(int v) { AppendBase(v); }
    void Append(double v) { AppendBase(v); }

    void Append(const char* v) { AppendBase(std::string(v)); }
    void Append(std::string_view v) { AppendBase(std::string(v)); }
    void Append(std::string&& v) { AppendBase(std::move(v)); }

    void Append(List&& v) { AppendBase(std::move(v)); }
    void Append(const List& v) { AppendBase(v); }

    void Append(Directory&& v) { AppendBase(std::move(v)); }
    void Append(const Directory& v) { AppendBase(v); }

    void Append(Value&& v) { AppendBase(std::move(v)); }
    void Append(const Value& v) { AppendBase(v); }

    iterator Remove(iterator itr) { return list_.erase(itr); }
    const_iterator Remove(const_iterator itr) { return list_.erase(itr); }

    void Clear() { list_.clear(); }
    std::size_t Size() const { return list_.size(); }

   private:
    template <typename T>
    void AppendBase(T&& v);

    std::vector<Value> list_;
};

class Directory {
   public:
    Directory() = default;

    Directory(Directory&&) = default;
    Directory(const Directory&) = default;
    Directory& operator=(Directory&&) = default;
    Directory& operator=(const Directory&) = default;

   public:
    using iterator = std::map<std::string, Value>::iterator;
    using const_iterator = std::map<std::string, Value>::const_iterator;

    iterator begin() { return map_.begin(); }
    const_iterator begin() const { return map_.begin(); }

    iterator end() { return map_.end(); }
    const_iterator end() const { return map_.end(); }

    const_iterator cbegin() const { return map_.cbegin(); }
    const_iterator cend() const { return map_.cend(); }

    inline void Iterate(std::function<bool(std::string_view, Value&)>);

    inline void Iterate(
        std::function<bool(std::string_view, const Value&)>) const;

    void Clear() { map_.clear(); }
    std::size_t Size() const { return map_.size(); }

   public:
    inline Value& operator[](std::string_view key);
    inline Value& operator[](const std::string& key) {
        return operator[](std::string_view(key));
    }
    inline Value& operator[](const char* key) {
        return operator[](std::string_view(key));
    }

    operator bool() const { return !map_.empty(); }

   public:
    Directory& Insert(std::string_view k, bool v) { return InsertBase(k, v); }
    Directory& Insert(std::string_view k, int v) { return InsertBase(k, v); }
    Directory& Insert(std::string_view k, double v) { return InsertBase(k, v); }

    Directory& Insert(std::string_view k, const char* v) {
        return InsertBase(k, std::string(v));
    }
    Directory& Insert(std::string_view k, std::string_view v) {
        return InsertBase(k, std::string(v));
    }
    Directory& Insert(std::string_view k, std::string&& v) {
        return InsertBase(k, std::move(v));
    }

    Directory& Insert(std::string_view k, List&& v) {
        return InsertBase(k, std::move(v));
    }
    Directory& Insert(std::string_view k, const List& v) {
        return InsertBase(k, v);
    }

    Directory& Insert(std::string_view k, Directory&& v) {
        return InsertBase(k, std::move(v));
    }
    Directory& Insert(std::string_view k, const Directory& v) {
        return InsertBase(k, v);
    }

    Directory& Insert(std::string_view k, Value&& v) {
        return InsertBase(k, std::move(v));
    }
    Directory& Insert(std::string_view k, const Value& v) {
        return InsertBase(k, v);
    }

    iterator Remove(iterator itr) { return map_.erase(itr); }
    const_iterator Remove(const_iterator itr) { return map_.erase(itr); }

    iterator Find(std::string_view key) { return map_.find(key); }

   public:
    bool InsertIfNotFound(std::string_view k, bool v) {
        return InsertIfNotFoundBase(k, v);
    }
    bool InsertIfNotFound(std::string_view k, int v) {
        return InsertIfNotFoundBase(k, v);
    }
    bool InsertIfNotFound(std::string_view k, double v) {
        return InsertIfNotFoundBase(k, v);
    }

    bool InsertIfNotFound(std::string_view k, const char* v) {
        return InsertIfNotFoundBase(k, std::string(v));
    }
    bool InsertIfNotFound(std::string_view k, std::string_view v) {
        return InsertIfNotFoundBase(k, std::string(v));
    }
    bool InsertIfNotFound(std::string_view k, std::string&& v) {
        return InsertIfNotFoundBase(k, std::move(v));
    }

    bool InsertIfNotFound(std::string_view k, List&& v) {
        return InsertIfNotFoundBase(k, std::move(v));
    }
    bool InsertIfNotFound(std::string_view k, const List& v) {
        return InsertIfNotFoundBase(k, v);
    }

    bool InsertIfNotFound(std::string_view k, Directory&& v) {
        return InsertIfNotFoundBase(k, std::move(v));
    }
    bool InsertIfNotFound(std::string_view k, const Directory& v) {
        return InsertIfNotFoundBase(k, v);
    }

    bool InsertIfNotFound(std::string_view k, Value&& v) {
        return InsertIfNotFoundBase(k, std::move(v));
    }
    bool InsertIfNotFound(std::string_view k, const Value& v) {
        return InsertIfNotFoundBase(k, v);
    }

   private:
    template <typename T>
    Directory& InsertBase(std::string_view key, T&& v);

    template <typename T>
    bool InsertIfNotFoundBase(std::string_view key, T&& v);

    // std::less<> mainly is used to compare between string_view and string
    // as a result, we can use string_view as key to insert and find elements
    std::map<std::string, Value, std::less<>> map_;
};

class Value {
   public:
    struct Null {};
    using Boolean = bool;
    using String = std::string;
    using Int = int;
    using Float = double;
    using Array = List;
    using Object = Directory;

    template <typename T>
    static inline constexpr bool IsValueType =
        std::is_same_v<T, Null> || std::is_same_v<T, Boolean> ||
        std::is_same_v<T, Int> || std::is_same_v<T, Float> ||
        std::is_same_v<T, String> || std::is_same_v<T, List> ||
        std::is_same_v<T, Directory>;

    enum Type : std::uint8_t {
        kNull = 0,
        kBoolean,
        kInt,
        kFloat,
        kString,
        kList,
        kDirectory,
    };

    // order matter
    using Storage =
        std::variant<Null, Boolean, Int, Float, String, List, Directory>;

   public:
    Value() = default;

    Value(Value&&) = default;
    Value(const Value&) = default;
    Value& operator=(Value&&) = default;
    Value& operator=(const Value&) = default;

    Value(bool v) : storage_(v) {}
    Value(int v) : storage_(v) {}
    Value(double v) : storage_(v) {}

    Value(const char* v) : storage_(std::string(v)) {}
    Value(std::string_view v) : storage_(std::string(v.begin(), v.end())) {}
    Value(std::string&& v) : storage_(std::move(v)) {}

    Value(List&& v) : storage_(std::move(v)) {}
    Value(const List& v) : storage_(v) {}

    Value(Directory&& v) : storage_(std::move(v)) {}
    Value(const Directory& v) : storage_(v) {}

    Value(const Null& null) : storage_(null) {}

   public:
    bool IsNull() const { return storage_.index() == kNull; }
    bool IsBoolean() const { return storage_.index() == kBoolean; }
    bool IsInt() const { return storage_.index() == kInt; }
    bool IsFloat() const { return storage_.index() == kFloat; }
    bool IsNumber() const { return IsInt() || IsFloat(); }
    bool IsString() const { return storage_.index() == kString; }
    bool IsList() const { return storage_.index() == kList; }
    bool IsDirectory() const { return storage_.index() == kDirectory; }

    void Clear() { storage_ = Null{}; }
    operator bool() const { return !IsNull(); }

   public:
    template <typename T>
    std::enable_if_t<IsValueType<T>, T*> To() {
        return std::get_if<T>(&storage_);
    }

    template <typename T>
    std::enable_if_t<IsValueType<T>, const T*> To() const {
        return std::get_if<T>(&storage_);
    }

    bool* ToBool() { return To<Boolean>(); }
    const bool* ToBool() const { return To<Boolean>(); }

    int* ToInt() { return To<Int>(); }
    const int* ToInt() const { return To<Int>(); }

    double* ToFloat() { return To<Float>(); }
    const double* ToFloat() const { return To<Float>(); }

    std::string* ToString() { return To<String>(); }
    const std::string* ToString() const { return To<String>(); }

    List* ToList() { return To<List>(); }
    const List* ToList() const { return To<List>(); }

    Directory* ToDirectory() { return To<Directory>(); }
    const Directory* ToDirectory() const { return To<Directory>(); }

   public:
    template <typename T>
    std::enable_if_t<IsValueType<T>, T&> As() {
        return std::get<T>(storage_);
    }

    template <typename T>
    std::enable_if_t<IsValueType<T>, const T&> As() const {
        return std::get<T>(storage_);
    }

    List& AsList(List&& lst) {
        storage_ = std::move(lst);
        return As<Array>();
    }
    List& AsList(const List& lst = {}) {
        storage_ = lst;
        return As<Array>();
    }

    Directory& AsDirectory(Directory&& dict) {
        storage_ = std::move(dict);
        return As<Object>();
    }
    Directory& AsDirectory(const Directory& dict = {}) {
        storage_ = dict;
        return As<Object>();
    }

    Null& AsNull() {
        storage_ = Null{};
        return As<Null>();
    }

    bool& AsBool(bool b = false) {
        storage_ = b;
        return As<Boolean>();
    }

    int& AsInt(int num = 0) {
        storage_ = num;
        return As<Int>();
    }

    double& AsFloat(double num = 0) {
        storage_ = num;
        return As<Float>();
    }

    std::string& AsString(const char* s) {
        return AsString(std::string_view(s));
    }

    std::string& AsString(std::string_view s) {
        storage_ = std::string(s.begin(), s.end());
        return As<String>();
    }

    std::string& AsString(std::string&& s = {}) {
        storage_ = std::move(s);
        return As<String>();
    }

    Type type() const { return static_cast<Type>(storage_.index()); }

   private:
    Storage storage_;
};

template <typename T>
void List::AppendBase(T&& v) {
    list_.emplace_back(std::forward<T>(v));
}

inline void List::Iteate(std::function<bool(Value&)> handler) {
    for (auto& elem : list_) {
        if (!handler(elem)) {
            return;
        }
    }
}

inline void List::Iteate(std::function<bool(const Value&)> handler) const {
    for (const auto& elem : list_) {
        if (!handler(elem)) {
            return;
        }
    }
}

template <typename T>
Directory& Directory::InsertBase(std::string_view key, T&& v) {
    if (auto itr = map_.find(key); itr != map_.end()) {
        itr->second = std::forward<T>(v);
    } else {
        map_.emplace(std::make_pair(key, std::forward<T>(v)));
    }

    return *this;
}

template <typename T>
bool Directory::InsertIfNotFoundBase(std::string_view key, T&& v) {
    auto [_, ok] = map_.emplace(std::make_pair(key, std::forward<T>(v)));
    return ok;
}

inline Value& Directory::operator[](std::string_view key) {
    // operator[] may insert a new element if key not found
    // however, the map::emplace method will insert a new element if key not
    // found, otherwise, do nothing.
    // for both of them, this method always returns a iterator pointing to value
    // corresponding to the specific key
    auto [itr, _] = map_.emplace(std::make_pair(key, Value{}));
    return itr->second;
}

inline void Directory::Iterate(
    std::function<bool(std::string_view, Value&)> handler) {
    for (auto& [key, v] : map_) {
        if (!handler(key, v)) {
            return;
        }
    }
}

inline void Directory::Iterate(
    std::function<bool(std::string_view, const Value&)> handler) const {
    for (const auto& [key, v] : map_) {
        if (!handler(key, v)) {
            return;
        }
    }
}

}  // namespace reflpp
