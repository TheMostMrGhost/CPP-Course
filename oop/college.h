#ifndef COLLEGE_H
#define COLLEGE_H

#include <cassert>
#include <memory>
#include <regex>
#include <set>

class Course;
class Person;
class Student;
class Teacher;
class PhDStudent;

// Design considerations:
// To minimise code reuse, we introduce class Academic which represents
// a person who can attend courses.
// This way Student and Teacher have very little code of their own.
// PhDStudent uses multiple interitance to be both a Student and a Teacher.
// To prevent inheriting from Person twice, Academic inherits virtually.
// Another problem is the dependency cycle between Course and Academic.
// Smart pointers utilising reference counting mustn't create cycles
// as that prevents memory from being freed.
// We use std::weak_ptr to break those cycles.

namespace col_detail {

template <typename T, typename... Ts>
concept same_as_any = (std::same_as<T, Ts> || ...);

template <typename T>
concept dereferenceable = requires(T a) { *a; };

struct deref_compare {
    template <typename T>
    bool
    operator()(std::weak_ptr<T> const &lhs, std::weak_ptr<T> const &rhs) const {
        auto locked_lhs = lhs.lock();
        auto locked_rhs = rhs.lock();

        if (!locked_lhs || !locked_rhs) {
            return !locked_lhs && locked_rhs;
        }

        return *locked_lhs < *locked_rhs;
    }

    template <dereferenceable T>
    bool operator()(T const &lhs, T const &rhs) const {
        return *lhs < *rhs;
    }
};

template <typename T>
using set_of_shp = std::set<std::shared_ptr<T>, deref_compare>;

template <typename T>
using set_of_wkp = std::set<std::weak_ptr<T>, deref_compare>;

}  // namespace col_detail

class Course {
public:
    Course(std::string_view name, bool active = true)
        : name(name), active(active) {}

    std::string const &get_name() const noexcept {
        return name;
    }

    bool is_active() const noexcept {
        return active;
    }

    auto operator<=>(Course const &other) const noexcept {
        return name <=> other.name;
    }

    bool operator==(Course const &other) const noexcept {
        return name == other.name;
    }

    void set_active(bool active) {
        this->active = active;
    }

    template <col_detail::same_as_any<Student, Teacher> T>
    bool add_participant(std::shared_ptr<T> const &person) {
        if (!person) {
            return false;
        }

        auto &set_of_T = std::get<col_detail::set_of_wkp<T>>(participants);
        auto it = set_of_T.find(person);

        if (it != set_of_T.end()) {
            auto locked_it = it->lock();

            if (locked_it && locked_it == person) {
                return false;
            }
        }

        if (!active) {
            throw std::logic_error("Incorrect operation for an inactive course.");
        }

        return set_of_T.insert(person).second;
    }

    template <col_detail::same_as_any<Student, Teacher> T>
    col_detail::set_of_shp<T> const get_participants() const {
        auto &set_of_T = std::get<col_detail::set_of_wkp<T>>(participants);
        col_detail::set_of_shp<T> res_participants;

        for (auto const &participant : set_of_T) {
            auto locked_participant = participant.lock();

            if (locked_participant) {
                res_participants.insert(locked_participant);
            }
        }

        return res_participants;
    }

private:
    std::string name;
    bool active;
    std::tuple<col_detail::set_of_wkp<Student>, col_detail::set_of_wkp<Teacher>>
        participants;
};

class Person {
public:
    Person(std::string_view name, std::string_view surname)
        : name(name), surname(surname) {}

    virtual ~Person() = default;

    std::string const &get_name() const noexcept {
        return name;
    }

    std::string const &get_surname() const noexcept {
        return surname;
    }

    auto operator<=>(Person const &other) const noexcept {
        auto surname_cmp = surname <=> other.surname;

        if (surname_cmp == 0) {
            return name <=> other.name;
        } else {
            return surname_cmp;
        }
    }

    bool operator==(Person const &other) const noexcept {
        return surname == other.surname && name == other.name;
    }

private:
    std::string name;
    std::string surname;
};

namespace col_detail {

// A set wrapper for courses which a person has.
class Academic : public virtual Person {
protected:
    Academic(std::string_view name, std::string_view surname)
        : Person(name, surname) {}
public:

    virtual ~Academic() = default;

    set_of_shp<Course const> const &get_courses() const noexcept {
        return courses;
    }

    virtual bool add_course(std::shared_ptr<Course> const &course) {
        if (!course) {
            return false;
        }

        return courses.insert(course).second;
    }

    void remove_course(std::shared_ptr<Course> const &course) noexcept {
        if (!course) {
            return;
        }

        courses.erase(course);
    }

protected:
    set_of_shp<Course const> courses;
};
}  // namespace col_detail

class Student : public col_detail::Academic {
public:
    Student(std::string_view name, std::string_view surname, bool active = true)
        : Person(name, surname), col_detail::Academic(name, surname),
          active(active) {}

    virtual ~Student() = default;

    bool is_active() const noexcept {
        return active;
    }

    void set_active(bool active) noexcept {
        this->active = active;
    }

    bool add_course(std::shared_ptr<Course> const &course) override {
        if (!course) {
            return false;
        }

        if (!active) {
            throw std::logic_error(
                "Incorrect operation for an inactive student."
            );
        }

        return courses.insert(course).second;
    }

private:
    bool active;
};

class Teacher : public col_detail::Academic {
public:
    Teacher(std::string_view name, std::string_view surname)
        : Person(name, surname), col_detail::Academic(name, surname) {}

    virtual ~Teacher() = default;
};

class PhDStudent : public Student, public Teacher {
public:
    PhDStudent(std::string_view name, std::string_view surname,
               bool active = true)
        : Person(name, surname), Student(name, surname, active),
          Teacher(name, surname) {}

    virtual ~PhDStudent() = default;
};

class College {
public:
    bool add_course(std::string_view name, bool active = true) {
        return courses.insert(std::make_shared<Course>(name, active)).second;
    }

    col_detail::set_of_shp<Course> const
    find_courses(std::string_view pattern) const {
        col_detail::set_of_shp<Course> found_courses;
        std::regex regex_pattern(wildcard_to_regex(pattern));

        for (auto const &course : courses) {
            if (std::regex_match(course->get_name(), regex_pattern)) {
                found_courses.insert(course);
            }
        }

        return found_courses;
    }

    bool change_course_activeness(std::shared_ptr<Course> const &course,
                                  bool active) const noexcept {
        if (!course) {
            return false;
        }

        auto it = courses.find(course);

        if (it == courses.end() || *it != course) {
            return false;
        }

        (*it)->set_active(active);
        return true;
    }

    bool remove_course(std::shared_ptr<Course> const &course) noexcept {
        if (!course) {
            return false;
        }

        auto it = courses.find(course);

        if (it == courses.end() || *it != course) {
            return false;
        }

        course->set_active(false);
        return courses.erase(course);
    }

    template <col_detail::same_as_any<Student, Teacher, PhDStudent> T>
    bool add_person(std::string_view name, std::string_view surname,
                    bool active = true) {
        if constexpr (std::is_same_v<T, Teacher>) {
            return participants.insert(std::make_shared<T>(name, surname))
                .second;
        } else {
            return participants
                .insert(std::make_shared<T>(name, surname, active))
                .second;
        }
    }

    bool change_student_activeness(std::shared_ptr<Student> const &student,
                                   bool active) const noexcept {
        if (!student) {
            return false;
        }

        auto it = participants.find(student);

        if (it == participants.end() || *it != student) {
            return false;
        }

        student->set_active(active);
        return true;
    }

    template <col_detail::same_as_any<Person, Student, Teacher, PhDStudent> T>
    col_detail::set_of_shp<T> const find(std::string_view name_pattern,
                                         std::string_view surname_pattern) const {
        col_detail::set_of_shp<T> found_persons;

        for (auto const &person : participants) {
            auto cast_person = std::dynamic_pointer_cast<T>(person);

            if (cast_person) {
                std::regex rx_name(wildcard_to_regex(name_pattern));
                std::regex rx_surname(wildcard_to_regex(surname_pattern));

                if (std::regex_match(cast_person->get_name(), rx_name)
                    && std::regex_match(
                        cast_person->get_surname(), rx_surname
                    )) {
                    found_persons.insert(cast_person);
                }
            }
        }

        return found_persons;
    }

    template <col_detail::same_as_any<Student, Teacher> T>
    col_detail::set_of_shp<T> const find(std::shared_ptr<Course> const &course) const {
        if (!course) {
            return {};
        }

        auto it = courses.find(course);

        if (it == courses.end() || *it != course) {
            return {};
        }

        return (*it)->get_participants<T>();
    }

    template <col_detail::same_as_any<Student, Teacher> T>
    bool assign_course(std::shared_ptr<T> const &person,
                       std::shared_ptr<Course> const &course) const {
        if (!person) {
            throw std::invalid_argument("Non-existing person.");
        }

        if (!course) {
            throw std::invalid_argument("Non-existing course.");
        }

        auto it = courses.find(course);

        if (it == courses.end() || *it != course) {
            throw std::invalid_argument("Non-existing course.");
        }

        if (!person_exists(person)) {
            throw std::invalid_argument("Non-existing person.");
        }

        if (!(*it)->is_active()) {
            throw std::logic_error("Incorrect operation on an inactive course.");
        }

        course->add_participant(person);
        bool result = person->add_course(course);

        return result;
    }

private:
    col_detail::set_of_shp<Course> courses;
    col_detail::set_of_shp<Person> participants;

    static std::string wildcard_to_regex(std::string_view pattern) {
        std::string regex_pattern;

        for (char c : pattern) {
            switch (c) {
            case '*':
                regex_pattern += ".*";
                break;
            case '?':
                regex_pattern += '.';
                break;
            case '.':
            case '+':
            case '\\':
            case '(':
            case ')':
            case '[':
            case ']':
            case '{':
            case '}':
            case '^':
            case '$':
            case '|':
                regex_pattern += '\\';
                regex_pattern += c;
                break;
            default:
                regex_pattern += c;
            }
        }

        return regex_pattern;
    }

    bool person_exists(std::shared_ptr<Person> const &person) const noexcept {
        if (!person) {
            return false;
        }

        auto it = participants.find(person);
        return it != participants.end() && *it == person;
    }
};

#endif  // COLLEGE_H
