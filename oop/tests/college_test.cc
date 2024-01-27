#include "college.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <sstream>

namespace {

#if TEST_NUM == 101
void test_101_example() {
  static char const * const example_log =
    "Jacek Chlebus\n"
    "Alicja Fiszer\n"
    "Jan Kowalski\n"
    "Pawel Kowalski\n"
    "Jakub Kubanski\n"
    "Kamil Litwiniuk\n"
    "Jan Kowalski\n"
    "Pawel Kowalski\n"
    "Jakub Kubanski\n"
    "Kamil Litwiniuk\n"
    "Algebra\n"
    "Analysis\n"
    "C++\n"
    "Incorrect operation on an inactive course.\n"
    "Incorrect operation for an inactive student.\n"
    "Incorrect operation on an inactive course.\n"
    "Non-existing person.\n"
    "Non-existing person.\n"
    "Non-existing course.\n";

  std::stringstream sout;

  // Test utworzony na podstawie przykładu z college_example.cc.
  Person *p1 = new Person("Janina", "Kowalska");
  Person *p2 = new Student("Piotr", "Nowak");
  Person *p3 = new Teacher("Wirgiliusz", "Kosobrudzki");
  Person *p4 = new PhDStudent("Cecylia", "Dorotea");

  delete p1;
  delete p2;
  delete p3;
  delete p4;

  // Tworzymy obiekt reprezentujący uczelnię i zarządzamy nią.
  College college;

  auto temp1 = college.add_course("Analysis");
  auto temp2 = college.add_course("Algebra");
  auto temp3 = college.add_course("Geometry");
  auto temp4 = college.add_course("C++");
  auto temp5 = college.add_course("Python");
  auto temp6 = college.add_course("History", false);
  auto temp7 = college.add_course("Biology");
  assert(temp1 && temp2 && temp3 && temp4 && temp5 && temp6 && temp7);
  assert(college.find_courses("*").size() == 7);

  auto courses = college.find_courses("C++");
  assert(courses.size() == 1);
  assert((*courses.begin())->get_name() == "C++");

  courses = college.find_courses("A*");
  assert(courses.size() == 2);
  auto it1 = courses.begin();
  assert((*it1)->get_name() == "Algebra");
  assert((*++it1)->get_name() == "Analysis");

  courses = college.find_courses("*o?y");
  assert(courses.size() == 2);
  it1 = courses.begin();
  assert((*it1)->get_name() == "Biology");
  assert((*++it1)->get_name() == "History");

  temp1 = college.add_person<Student>("Jan", "Kowalski");
  temp2 = college.add_person<Student>("Pawel", "Kowalski", false);
  temp3 = college.add_person<Teacher>("Jacek", "Chlebus");
  temp4 = college.add_person<Teacher>("Kamil", "Litwiniuk");
  temp5 = college.add_person<PhDStudent>("Alicja", "Fiszer");
  temp6 = college.add_person<PhDStudent>("Jakub", "Kubanski");
  assert(temp1 && temp2 && temp3 && temp4 && temp5 && temp6);

  assert(!college.add_person<Teacher>("Jan", "Kowalski"));
  assert(!college.add_person<Student>("Jan", "Kowalski"));
  assert(!college.add_person<Student>("Jan", "Kowalski", true));
  assert(!college.add_person<Student>("Jan", "Kowalski", false));
  assert(!college.add_person<PhDStudent>("Jan", "Kowalski"));
  assert(!college.add_person<Teacher>("Jan", "Kowalski"));
  assert(!college.add_person<Teacher>("Alicja", "Fiszer"));
  assert(!college.add_person<PhDStudent>("Jacek", "Chlebus"));

  auto people = college.find<Person>("*", "*");
  assert(people.size() == 6);
  assert((*people.begin())->get_name() == "Jacek");
  assert((*people.begin())->get_surname() == "Chlebus");

  for (auto const &p : people)
    sout << p->get_name() << ' ' << p->get_surname() << '\n';

  people = college.find<Person>("*", "*k*");
  assert(people.size() == 4);

  for (auto const &p : people)
    sout << p->get_name() << ' ' << p->get_surname() << '\n';

  assert(college.find<Student>("*", "*").size() == 4);
  assert(college.find<PhDStudent>("*", "*").size() == 2);
  assert(college.find<Teacher>("*", "*").size() == 4);

  auto jan_kowalski = *college.find<Student>("Jan", "Kowalski").begin();
  auto pawel_kowalski = *college.find<Student>("Pawel", "Kowalski").begin();
  auto alicja_fiszer = *college.find<PhDStudent>("Alicja", "Fiszer").begin();
  auto jakub_kubanski = *college.find<PhDStudent>("Jakub", "Kubanski").begin();
  auto jacek_chlebus = *college.find<Teacher>("Jacek", "Chlebus").begin();
  auto cxx = *college.find_courses("C++").begin();
  auto algebra = *college.find_courses("Algebra").begin();
  auto analysis = *college.find_courses("Analysis").begin();
  auto geometry = *college.find_courses("Geometry").begin();
  auto history = *college.find_courses("History").begin();

  temp1 = college.assign_course(jan_kowalski, cxx);
  temp2 = college.assign_course(jan_kowalski, algebra);
  temp3 = college.assign_course(jan_kowalski, analysis);
  temp4 = college.assign_course<Student>(alicja_fiszer, cxx);
  temp5 = college.assign_course<Teacher>(jakub_kubanski, cxx);
  temp6 = college.assign_course(jacek_chlebus, cxx);
  temp7 = college.assign_course(jacek_chlebus, algebra);
  auto temp8 = college.assign_course(jacek_chlebus, analysis);
  assert(temp1 && temp2 && temp3 && temp4 && temp5 && temp6 && temp7 && temp8);

  assert(college.find<Student>(cxx).size() == 2);
  assert(college.find<Student>(algebra).size() == 1);
  assert(college.find<Student>(geometry).size() == 0);
  assert(college.find<Teacher>(cxx).size() == 2);
  assert(college.find<Teacher>(algebra).size() == 1);
  assert(college.find<Teacher>(history).size() == 0);

  assert(alicja_fiszer->Teacher::get_courses().empty());
  auto jacek_courses = jacek_chlebus->get_courses();
  assert((*jacek_courses.begin())->get_name() == "Algebra");

  for (auto const &c : jacek_courses)
    sout << c->get_name() << '\n';

  assert(algebra->is_active() == true);
  college.change_course_activeness(algebra, false);
  assert(algebra->is_active() == false);

  assert(jan_kowalski->is_active() == true);
  college.change_student_activeness(jan_kowalski, false);
  assert(jan_kowalski->is_active() == false);

  try {
    college.assign_course(jan_kowalski, history);
    assert(false);
  } catch (std::exception const & e) {
    sout << e.what() << std::endl;
  }

  try {
    college.assign_course(pawel_kowalski, cxx);
    assert(false);
  } catch (std::exception const & e) {
    sout << e.what() << std::endl;
  }

  try {
    college.assign_course(jacek_chlebus, history);
    assert(false);
  } catch (std::exception const & e) {
    sout << e.what() << std::endl;
  }

  try {
    auto jack = std::make_shared<Student>("Jack", "London");
    college.assign_course(jack, cxx);
    assert(false);
  } catch (std::exception const & e) {
    sout << e.what() << std::endl;
  }

  try {
    auto jan_kowalski2 = std::make_shared<Student>("Jan", "Kowalski");
    college.assign_course(jan_kowalski2, cxx);
    assert(false);
  } catch (std::exception const & e) {
    sout << e.what() << std::endl;
  }

  try {
    auto cxx2 = std::make_shared<Course>("C++");
    college.assign_course(jan_kowalski, cxx2);
    assert(false);
  } catch (std::exception const & e) {
    sout << e.what() << std::endl;
  }

  assert((*alicja_fiszer->Student::get_courses().begin())->get_name() == "C++");
  assert(cxx->is_active() == true);
  assert(college.remove_course(cxx));
  assert(!college.change_course_activeness(cxx, true));
  assert(cxx->is_active() == false);
  assert(college.find_courses("C++").empty());
  assert((*alicja_fiszer->Student::get_courses().begin())->get_name() == "C++");
  assert(jacek_chlebus->get_courses().count(cxx) == 1);

  assert(sout.view() == example_log);
}
#endif // TEST_NUM == 101

#if TEST_NUM == 103
void test_103_forum() {
  College c;
  assert(c.add_course("PW"));
  assert(c.add_person<Student>("A", "A"));
  auto s = *c.find<Student>("A", "A").begin();
  auto p = *c.find_courses("PW").begin();
  assert(c.assign_course(s, p));
  assert(c.remove_course(p));
  assert(c.add_course("PW"));
  auto p2 = *c.find_courses("PW").begin();
  assert(!c.assign_course(s, p2));
}
#endif // TEST_NUM == 103

#if TEST_NUM == 104
void test_104_example_2() {
  College c1, c2;
  assert(c1.add_course("Analysis"));
  assert(c2.add_course("Analysis"));
  assert(c1.add_person<Student>("Jan", "Kowalski"));
  assert(c2.add_person<Student>("Piotr", "Nowak"));
  auto jan_kowalski = *c1.find<Student>("Jan", "Kowalski").begin();
  auto piotr_nowak = *c2.find<Student>("Piotr", "Nowak").begin();
  auto analysis1 = *c1.find_courses("Analysis").begin();
  auto analysis2 = *c2.find_courses("Analysis").begin();
  assert(c1.assign_course(jan_kowalski, analysis1));
  assert(c2.assign_course(piotr_nowak, analysis2));
  assert(c1.find<Student>(analysis1).size() == 1);
  assert(c2.find<Student>(analysis2).size() == 1);
  assert(c1.find<Student>(analysis2).size() == 0);
  assert(c2.find<Student>(analysis1).size() == 0);
}
#endif // TEST_NUM == 104

#if TEST_NUM == 201
void test_201_Course() {
  Course cxx("C++");
  assert(cxx.get_name() == "C++" && cxx.is_active());

  Course biology("Biology", false);
  assert(biology.get_name() == "Biology" && !biology.is_active());

  College college;
  College college2;

  auto temp1 = college.add_course("course1");
  auto temp2 = college.add_course("course2", false);
  auto temp3 = college.add_course("course1a", true);
  auto temp4 = college.add_course("course0");
  auto temp5 = college.add_course("course1b");
  assert(temp1 && temp2 && temp3 && temp4 && temp5);

  auto courses = college.find_courses("*");
  assert(courses.size() == 5);
  auto it = courses.begin();
  assert((*it)->get_name() == "course0");
  assert((*it)->is_active());
  it++;
  assert((*it)->get_name() == "course1");
  it++;
  assert((*it)->get_name() == "course1a");
  assert((*it)->is_active());
  auto course1a = *it;
  it++;
  assert((*it)->get_name() == "course1b");
  auto course1b = *it;
  it++;
  assert((*it)->get_name() == "course2");
  assert(!(*it)->is_active());
  auto course2 = *it;

  assert(!college.add_course("course1"));
  assert(!college.add_course("course1", true));
  assert(!college.add_course("course1", false));
  assert(!college.add_course("course2"));
  assert(!college.add_course("course2", true));
  assert(!college.add_course("course2", false));
  assert(!college.add_course("course1a"));
  assert(!college.add_course("course1a", true));
  assert(!college.add_course("course1a", false));

  assert(college2.add_course("course1"));
  assert(*college.find_courses("course1").begin() !=
         *college2.find_courses("course1").begin());

  assert(college.find_courses("c*").size() == 5);
  assert(college.find_courses("*ou*1*").size() == 3);
  assert(college.find_courses("*ou*1?").size() == 2);
  assert(college.find_courses("course?").size() == 3);
  assert(college.find_courses("??*s??a*").size() == 1);
  assert(college.find_courses("??*s?*").size() == 5);

  courses = college.find_courses("??*s???");
  assert(courses.size() == 2);
  it = courses.begin();
  assert(*it == course1a);
  assert(*++it == course1b);

  assert(college.change_course_activeness(course1a, false));
  assert(!course1a->is_active());
  assert(college.change_course_activeness(course1a, true));
  assert(course1a->is_active());

  temp1 = college.add_person<Student>("I1", "N1");
  temp2 = college.add_person<Teacher>("I2", "N2");
  temp3 = college.add_person<PhDStudent>("I3", "N3");
  assert(temp1 && temp2 && temp3);
  auto person1 = *college.find<Student>("I1", "N1").begin();
  auto person2 = *college.find<Teacher>("I2", "N2").begin();
  auto person3 = *college.find<PhDStudent>("I3", "N3").begin();
  temp1 = college.assign_course(person1, course1a);
  temp2 = college.assign_course(person2, course1b);
  assert(temp1 && temp2);
  temp1 = college.assign_course<Student>(person3, course1a);
  temp2 = college.assign_course<Teacher>(person3, course1b);
  assert(temp1 && temp2);

  temp1 = college.remove_course(course1a);
  temp2 = college.remove_course(course1b);
  assert(temp1 && temp2);
  auto course3 = std::make_shared<Course>("C++");
  assert(!college.remove_course(course3));

  assert(!course1a->is_active());
  assert(!course1b->is_active());
  assert(course3->is_active());

  assert(*person1->get_courses().begin() == course1a);
  assert(*person2->get_courses().begin() == course1b);
  assert(*person3->Student::get_courses().begin() == course1a);
  assert(*person3->Teacher::get_courses().begin() == course1b);
}
#endif // TEST_NUM == 201

#if TEST_NUM == 202
void test_202_Course() { // It does not compile.
  Course course;
}
#endif // TEST_NUM == 202

#if TEST_NUM == 203
void test_203_Course() { // It does not compile.
  Course course("C");
  course.get_name() = "C++";
}
#endif // TEST_NUM == 203

#if TEST_NUM == 301
void test_301_Person() {
  Person person1("I1", "N1");
  assert(person1.get_name() == "I1");
  assert(person1.get_surname() == "N1");

  College college;
  college.add_person<Student>("Jan", "Kowalski");
  college.add_person<Student>("Pawel", "Kowalski", false);
  college.add_person<Teacher>("Jacek", "Chlebus");
  college.add_person<Teacher>("Kamil", "Litwiniuk");
  college.add_person<PhDStudent>("Alicja", "Fiszer");
  college.add_person<PhDStudent>("Jakub", "Kubanski");

  auto jan_kowalski = *college.find<Person>("Jan", "Kowalski").begin();
  auto pawel_kowalski = *college.find<Person>("Pawel", "Kowalski").begin();
  auto jacek_chlebus = *college.find<Person>("Jacek", "Chlebus").begin();
  auto kamil_litwiniuk = *college.find<Person>("Kamil", "Litwiniuk").begin();
  auto alicja_fiszer = *college.find<Person>("Alicja", "Fiszer").begin();
  auto jakub_kubanski = *college.find<Person>("Jakub", "Kubanski").begin();

  assert(jan_kowalski->get_name() == "Jan" &&
         jan_kowalski->get_surname() == "Kowalski");
  assert(jacek_chlebus->get_name() == "Jacek" &&
         jacek_chlebus->get_surname() == "Chlebus");
  assert(alicja_fiszer->get_name() == "Alicja" &&
         alicja_fiszer->get_surname() == "Fiszer");

  auto jan_kowalski2 = std::dynamic_pointer_cast<Student, Person>(jan_kowalski);
  auto jacek_chlebus2 = std::dynamic_pointer_cast<Teacher, Person>(jacek_chlebus);
  auto alicja_fiszer2 = std::dynamic_pointer_cast<PhDStudent, Person>(alicja_fiszer);

  assert(jan_kowalski2->get_name() == "Jan" &&
         jan_kowalski2->get_surname() == "Kowalski");
  assert(jacek_chlebus2->get_name() == "Jacek" &&
         jacek_chlebus2->get_surname() == "Chlebus");
  assert(alicja_fiszer2->get_name() == "Alicja" &&
         alicja_fiszer2->get_surname() == "Fiszer");

  auto people = college.find<Person>("?a*", "?o*");
  assert(people.size() == 2);
  auto it = people.begin();
  assert(*it == jan_kowalski);
  assert(*++it == pawel_kowalski);

  people = college.find<Person>("*c??", "*");
  assert(people.size() == 2);
  it = people.begin();
  assert(*it == jacek_chlebus);
  assert(*++it == alicja_fiszer);

  assert(*college.find<Person>("*c*", "*u?").begin() == jacek_chlebus);
  assert(*college.find<Person>("?a*e?", "K*k?").begin() == pawel_kowalski);
}
#endif // TEST_NUM == 301

#if TEST_NUM == 302
void test_302_Person() { // It does not compile.
  Person person;
}
#endif // TEST_NUM == 302

#if TEST_NUM == 303
void test_303_Person() { // It does not compile.
  Person person("I", "N");
  person.get_name() = "A";
}
#endif // TEST_NUM == 303

#if TEST_NUM == 304
void test_304_Person() { // It does not compile.
  Person person("I", "N");
  person.get_surname() = "B";
}
#endif // TEST_NUM == 304

#if TEST_NUM == 305
void test_305_Person() { // It does not compile.
  College college;
  college.add_person<Person>("Jan", "Kowalski");
}
#endif // TEST_NUM == 305

#if TEST_NUM == 401
void test_401() {
  static char const * const test_log =
    "Incorrect operation for an inactive student.\n"
    "Incorrect operation for an inactive student.\n"
    "Incorrect operation on an inactive course.\n"
    "Incorrect operation on an inactive course.\n"
    "Non-existing course.\n"
    "Non-existing course.\n"
    "Non-existing course.\n"
    "Non-existing course.\n"
    "Non-existing person.\n"
    "Non-existing person.\n"
    "Non-existing person.\n"
    "Non-existing person.\n"
    "Non-existing course.\n"
    "Non-existing course.\n"
    "Non-existing course.\n"
    "Non-existing course.\n";

  std::stringstream sout;

  auto student1 = std::make_shared<Student>("I1", "N1");
  auto student2 = std::make_shared<Student>("I2", "N2", true);
  auto student3 = std::make_shared<Student>("I3", "N3", false);
  assert(student1->get_name() == "I1" && student1->get_surname() == "N1");
  assert(student1->is_active() && student2->is_active() && !student3->is_active());
  auto person1 = std::dynamic_pointer_cast<Person, Student>(student1);
  assert(person1->get_name() == "I1" && person1->get_surname() == "N1");

  auto teacher1 = std::make_shared<Teacher>("A1", "B1");
  assert(teacher1->get_name() == "A1" && teacher1->get_surname() == "B1");
  auto person2 = std::dynamic_pointer_cast<Person, Teacher>(teacher1);
  assert(person2->get_name() == "A1" && person2->get_surname() == "B1");

  auto phd_student1 = std::make_shared<PhDStudent>("C1", "D1");
  assert(phd_student1->get_name() == "C1" &&
         phd_student1->get_surname() == "D1" &&
         phd_student1->is_active());
  auto person3a = std::dynamic_pointer_cast<Person, PhDStudent>(phd_student1);
  auto person3b = std::dynamic_pointer_cast<Student, PhDStudent>(phd_student1);
  auto person3c = std::dynamic_pointer_cast<Teacher, PhDStudent>(phd_student1);
  assert(person3a->get_name() == "C1" && person3a->get_surname() == "D1");
  assert(person3b->get_name() == "C1" && person3b->get_surname() == "D1");
  assert(person3c->get_name() == "C1" && person3c->get_surname() == "D1");

  College college;
  college.add_person<Student>("I1", "N1");
  college.add_person<Student>("I0", "N0", false);
  college.add_person<Student>("I1", "N0");
  college.add_person<Student>("I0", "N1");
  college.add_person<Teacher>("A1", "B1");
  college.add_person<Teacher>("A0", "B1");
  college.add_person<Teacher>("A1", "B0", true);
  college.add_person<Teacher>("A0", "B0", false);
  college.add_person<PhDStudent>("C0", "D1");
  college.add_person<PhDStudent>("C1", "D0");
  college.add_person<PhDStudent>("C0", "D0", false);
  college.add_person<PhDStudent>("C1", "D1", true);

  auto student_1_1 = *college.find<Student>("I1", "N1").begin();
  auto student_0_0 = *college.find<Student>("I0", "N0").begin();
  auto teacher_1_1 = *college.find<Teacher>("A1", "B1").begin();
  auto teacher_0_0 = *college.find<Teacher>("A0", "B0").begin();
  auto phd_student_1_1 = *college.find<PhDStudent>("C1", "D1").begin();
  auto phd_student_0_0 = *college.find<PhDStudent>("C0", "D0").begin();

  assert(student_1_1 != student1 &&
         teacher_1_1 != teacher1 &&
         phd_student_1_1 != phd_student1);

  assert(student_1_1->get_name() == "I1" && student_1_1->get_surname() == "N1");
  assert(teacher_1_1->get_name() == "A1" && teacher_1_1->get_surname() == "B1");
  assert(phd_student_1_1->get_name() == "C1" &&
         phd_student_1_1->get_surname() == "D1");

  assert(student_1_1->is_active() && !student_0_0->is_active());
  college.change_student_activeness(student_0_0, true);
  assert(student_0_0->is_active());
  college.change_student_activeness(student_0_0, false);
  assert(!student_0_0->is_active());

  assert(phd_student_1_1->is_active() && !phd_student_0_0->is_active());
  college.change_student_activeness(phd_student_0_0, true);
  assert(phd_student_0_0->is_active());
  college.change_student_activeness(phd_student_0_0, false);
  assert(!phd_student_0_0->is_active());

  auto s_1_1 = std::dynamic_pointer_cast<Person, Student>(student_1_1);
  auto t_1_1 = std::dynamic_pointer_cast<Person, Teacher>(teacher_1_1);
  auto pa_1_1 = std::dynamic_pointer_cast<Person, PhDStudent>(phd_student_1_1);
  auto pb_1_1 = std::dynamic_pointer_cast<Student, PhDStudent>(phd_student_1_1);
  auto pc_1_1 = std::dynamic_pointer_cast<Teacher, PhDStudent>(phd_student_1_1);
  assert(s_1_1->get_name() == "I1" && s_1_1->get_surname() == "N1");
  assert(t_1_1->get_name() == "A1" && t_1_1->get_surname() == "B1");
  assert(pa_1_1->get_name() == "C1" && pa_1_1->get_surname() == "D1");
  assert(pb_1_1->get_name() == "C1" && pb_1_1->get_surname() == "D1");
  assert(pc_1_1->get_name() == "C1" && pc_1_1->get_surname() == "D1");
  assert(pb_1_1->is_active());

  assert(college.find<Person>("*", "*").size() == 12);
  assert(college.find<Student>("*", "*").size() == 8);
  assert(college.find<Teacher>("*", "*").size() == 8);
  assert(college.find<PhDStudent>("*", "*").size() == 4);

  assert(*college.find<Person>("*", "*").begin() == teacher_0_0);
  assert(*college.find<Person>("I*", "*").begin() == student_0_0);
  assert(*college.find<Person>("*", "D*").begin() == phd_student_0_0);
  assert(*college.find<Person>("*1", "*1").begin() == teacher_1_1);

  assert(*college.find<Student>("*", "*").begin() == phd_student_0_0);
  assert(*college.find<Student>("I*", "*").begin() == student_0_0);
  assert(*college.find<Student>("?1", "?1").begin() == phd_student_1_1);
  assert(*college.find<Student>("*1", "N*1").begin() == student_1_1);

  assert(*college.find<Teacher>("*", "*").begin() == teacher_0_0);
  assert(*college.find<Teacher>("?1", "B*1").begin() == teacher_1_1);
  assert(*college.find<Teacher>("C*", "*").begin() == phd_student_0_0);
  assert(*college.find<Teacher>("C*1", "?1").begin() == phd_student_1_1);

  assert(*college.find<PhDStudent>("*", "*").begin() == phd_student_0_0);
  assert(*college.find<PhDStudent>("*1", "*1").begin() == phd_student_1_1);

  college.add_course("course1");
  college.add_course("course2");
  college.add_course("course3");
  college.add_course("course4", false);
  auto course1 = *college.find_courses("course1").begin();
  auto course2 = *college.find_courses("course2").begin();
  auto course3 = *college.find_courses("course3").begin();
  auto course4 = *college.find_courses("course4").begin();
  auto course5 = std::make_shared<Course>("course5");

  college.change_student_activeness(student_0_0, true);
  college.assign_course(student_1_1, course1);
  college.assign_course(student_1_1, course2);
  college.assign_course(student_0_0, course1);
  college.assign_course(student_0_0, course2);

  college.assign_course(teacher_1_1, course1);
  college.assign_course(teacher_0_0, course1);
  college.assign_course(teacher_0_0, course2);
  college.assign_course(teacher_1_1, course2);

  college.assign_course<Student>(phd_student_1_1, course1);
  college.assign_course<Teacher>(phd_student_1_1, course2);

  auto people = college.find<Student>(course1);
  auto it = people.begin();
  assert(*it == phd_student_1_1);
  assert(*++it == student_0_0);
  assert(*++it == student_1_1);

  people = college.find<Student>(course2);
  it = people.begin();
  assert(*it == student_0_0);
  assert(*++it == student_1_1);

  assert(college.find<Teacher>(course1).size() == 2);
  auto people2 = college.find<Teacher>(course2);
  auto it2 = people2.begin();
  assert(*it2 == teacher_0_0);
  assert(*++it2 == teacher_1_1);
  assert(*++it2 == phd_student_1_1);

  college.change_student_activeness(student_0_0, false);
  college.remove_course(course1);
  college.remove_course(course2);
  for (char c = 0; c < 16; c++) {
    try {
      switch (c) {
        case 0:
          college.assign_course(student_0_0, course3);
          break;
        case 1:
          college.assign_course<Student>(phd_student_0_0, course3);
          break;
        case 2:
          college.assign_course(teacher_1_1, course4);
          break;
        case 3:
          college.assign_course<Teacher>(phd_student_1_1, course4);
          break;
        case 4:
          college.assign_course(student_1_1, course5);
          break;
        case 5:
          college.assign_course<Student>(phd_student_1_1, course5);
          break;
        case 6:
          college.assign_course(teacher_1_1, course5);
          break;
        case 7:
          college.assign_course<Teacher>(phd_student_1_1, course5);
          break;
        case 8:
          college.assign_course(student1, course3);
          break;
        case 9:
          college.assign_course<Student>(phd_student1, course3);
          break;
        case 10:
          college.assign_course(teacher1, course3);
          break;
        case 11:
          college.assign_course<Teacher>(phd_student1, course3);
          break;
        case 12:
          college.assign_course(student_1_1, course1);
          break;
        case 13:
          college.assign_course<Student>(phd_student_1_1, course1);
          break;
        case 14:
          college.assign_course(teacher_1_1, course2);
          break;
        case 15:
          college.assign_course<Teacher>(phd_student_1_1, course2);
          break;
      }
      assert(false);
    } catch (std::exception const & e) {
      sout << e.what() << std::endl;
    }
  }
  assert(sout.view() == test_log);
}
#endif // TEST_NUM == 401

#if TEST_NUM == 402
void test_402() { // It does not compile.
  Student student;
}
#endif // TEST_NUM == 402

#if TEST_NUM == 403
void test_403() { // It does not compile.
  Teacher teacher;
}
#endif // TEST_NUM == 403

#if TEST_NUM == 404
void test_404() { // It does not compile.
  PhDStudent phd_student;
}
#endif // TEST_NUM == 404

#if TEST_NUM >= 405 && TEST_NUM <= 408
void test_405_408() { // It does not compile.
  College college;
  college.add_course("course1");
  auto course1 = *college.find_courses("course1").begin();
  college.add_course("course2");
  auto course2 = *college.find_courses("course2").begin();

  #if TEST_NUM == 405
  college.add_person<Student>("I", "N");
  auto person = *college.find<Student>("I", "N").begin();
  #elif TEST_NUM == 406
  college.add_person<Teacher>("I", "N");
  auto person = *college.find<Teacher>("I", "N").begin();
  #else
  college.add_person<PhDStudent>("I", "N");
  auto person = *college.find<PhDStudent>("I", "N").begin();
  #endif

  #if TEST_NUM == 405 || TEST_NUM == 406
  college.assign_course(person, course1);
  auto & courses = person->get_courses();
  #elif TEST_NUM == 407
  college.assign_course<Student>(person, course1);
  auto & courses = person->Student::get_courses();
  #else
  college.assign_course<Teacher>(person, course1);
  auto & courses = person->Teacher::get_courses();
  #endif

  courses.insert(course2);
}
#endif

#if TEST_NUM == 501
void test_501() {
  auto person = std::make_shared<PhDStudent>("I", "N");
  auto person2 = std::dynamic_pointer_cast<Student, PhDStudent>(person);
  auto person3 = std::dynamic_pointer_cast<Teacher, PhDStudent>(person);
  assert(&person2->get_name() == &person3->get_name() &&
         &person2->get_surname() == &person3->get_surname());

  College college;
  college.add_person<PhDStudent>("I", "N");
  auto phd_student = *college.find<PhDStudent>("I", "N").begin();
  college.add_course("course");
  auto course = *college.find_courses("course").begin();
  assert(college.assign_course<Student>(phd_student, course));
  assert(college.assign_course<Teacher>(phd_student, course));
  assert(&phd_student->Student::get_courses() !=
         &phd_student->Teacher::get_courses());

  college.remove_course(course);
  college.add_course("course");
  auto course2 = *college.find_courses("course").begin();
  assert(course2 != course && !course->is_active() && course2->is_active());
  assert(!college.assign_course<Student>(phd_student, course2));
  assert(!college.assign_course<Teacher>(phd_student, course2));
}
#endif // TEST_NUM == 501

#if TEST_NUM == 502
void test_502() {
  College college, college2;

  college.add_person<Student>("I", "N");
  college2.add_person<Student>("I", "N");
  college.add_person<Teacher>("A", "B");
  college2.add_person<Teacher>("A", "B");
  college.add_person<PhDStudent>("C", "D");
  college2.add_person<PhDStudent>("C", "D");

  auto student = *college.find<Student>("I", "N").begin();
  auto student2 = *college2.find<Student>("I", "N").begin();
  auto teacher = *college.find<Teacher>("A", "B").begin();
  auto teacher2 = *college2.find<Teacher>("A", "B").begin();
  auto phd_student = *college.find<PhDStudent>("C", "D").begin();
  auto phd_student2 = *college2.find<PhDStudent>("C", "D").begin();

  assert(student != student2 &&
         teacher != teacher2 &&
         phd_student != phd_student2);

  college.add_course("course");
  college2.add_course("course");
  auto course = *college.find_courses("course").begin();
  auto course2 = *college2.find_courses("course").begin();

  assert(course != course2);

  assert(college2.change_student_activeness(student2, false));
  assert(college2.change_student_activeness(phd_student2, false));
  assert(student->is_active() == true);
  assert(student2->is_active() == false);
  assert(phd_student->is_active() == true);
  assert(phd_student2->is_active() == false);

  assert(college2.change_course_activeness(course2, false));
  assert(course->is_active() == true);
  assert(course2->is_active() == false);
}
#endif // TEST_NUM == 502

#if TEST_NUM == 503 || TEST_NUM == 504
void test_503_504() { // It does not compile.
  College college;
  college.add_person<PhDStudent>("I", "N");
  auto phd_student = *college.find<PhDStudent>("I", "N").begin();
  college.add_course("course");
  auto course = *college.find_courses("course").begin();

  #if TEST_NUM == 503
  college.assign_course<PhDStudent>(phd_student, course);
  #else
  [[maybe_unused]] auto temp = college.find<PhDStudent>(course);
  #endif
}
#endif

#if TEST_NUM == 505
void test_505() { // It does not compile.
  College college;
  auto temp = college.find<Course>("*", "*");
}
#endif // TEST_NUM == 504

#if TEST_NUM == 601
void test_601() { // Testing performance
  College college;
  char buf1[24], buf2[24];
  const int n = 550;
  srand(0);
  for (int i = 0; i < n; i++) {
    sprintf(buf1, "%d_%d", i, rand());
    sprintf(buf2, "%d_%d", i, rand());
    college.add_person<Student>(buf1, buf2);
  }
  srand(0);
  for (int i = 0; i < n; i++) {
    int len1 = sprintf(buf1, "%d_%d*", i, rand());
    int len2 = sprintf(buf2, "%d_%d*", i, rand());
    auto it = *college.find<Student>(buf1, buf2).begin();
    buf1[len1 - 1] = 0;
    buf2[len2 - 1] = 0;
    assert(it->get_name() == buf1 && it->get_surname() == buf2);
  }
}
#endif // TEST_NUM == 601

#if TEST_NUM == 602
void test_602() { // Testing performance
  College college;
  char buf[24];
  const int n = 475;
  for (int i = 0; i < n; i++) {
    sprintf(buf, "%020d", i);
    college.add_course(buf);
  }
  for (int i = 0; i < n; i++) {
    int len = sprintf(buf, "%020d*", i);
    auto it = *college.find_courses(buf).begin();
    buf[len - 1] = 0;
    assert(it->get_name() == buf);
  }
}
#endif // TEST_NUM == 602

} // koniec anonimowej przestrzeni nazw

int main() {
#if TEST_NUM == 101
  test_101_example();
#elif TEST_NUM == 102
  // Test 102 jest w pliku college_test_external.cc.
#elif TEST_NUM == 103
  test_103_forum();
#elif TEST_NUM == 104
  test_104_example_2();
#elif TEST_NUM == 201
  test_201_Course();
#elif TEST_NUM == 202
  test_202_Course();
#elif TEST_NUM == 203
  test_203_Course();
#elif TEST_NUM == 301
  test_301_Person();
#elif TEST_NUM == 302
  test_302_Person();
#elif TEST_NUM == 303
  test_303_Person();
#elif TEST_NUM == 304
  test_304_Person();
#elif TEST_NUM == 305
  test_305_Person();
#elif TEST_NUM == 401
  test_401();
#elif TEST_NUM == 402
  test_402();
#elif TEST_NUM == 403
  test_403();
#elif TEST_NUM == 404
  test_404();
#elif TEST_NUM >= 405 && TEST_NUM <= 408
  test_405_408();
#elif TEST_NUM == 501
  test_501();
#elif TEST_NUM == 502
  test_502();
#elif TEST_NUM == 503 || TEST_NUM == 504
  test_503_504();
#elif TEST_NUM == 505
  test_505();
#elif TEST_NUM == 601
  test_601();
#elif TEST_NUM == 602
  test_602();
#endif
}
