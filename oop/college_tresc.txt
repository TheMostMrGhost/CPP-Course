Zadanie polega na zaprojektowaniu i zaimplementowaniu klas do zarządzania
uczelnią w bardzo uproszczonej formie. Oczekiwane są przynajmniej następujące
klasy.

== Course ==

Klasa Course reprezentuje przedmiot. Przedmiot ma nazwę i atrybut typu bool
określający, czy przedmiot jest aktywny.

Klasa Course ma konstruktor przyjmujący dwa parametry: nazwa, aktywność.
Domyślną wartością aktywności jest true.

Klasa Course nie ma konstruktora bezparametrowego.

Klasa Course ma metody get_name i is_active dające dostęp do jej atrybutów.

Zakładamy, że kursy są jednoznacznie identyfikowane przez ich nazwy.

== Person ==

Klasa Person reprezentuje osobę. Osoba ma imię i nazwisko.

Klasa Person ma konstruktor przyjmujący dwa parametry: imię, nazwisko.

Klasa Person nie ma konstruktora bezparametrowego.

Klasa Person ma metody get_name i get_surname dające dostęp do jej atrybutów.

Zakładamy, że osoby są jednoznacznie identyfikowane imieniem i nazwiskiem.

== Student ==

Klasa Student reprezentuje studenta. Student jest osobą. Student ma listę
przedmiotów, na które jest zapisany. Student może być aktywny lub nieaktywny.

Klasa Student ma konstruktor przyjmujący trzy parametry: imię, nazwisko,
aktywność. Domyślną wartością aktywności jest true.

Klasa Student nie ma konstruktora bezparametrowego.

Klasa Student ma metodę is_active dającą informację, czy student jest aktywny.

Klasa Student ma metodę get_courses dającą niemodyfikowalny dostęp do jego
przedmiotów posortowanych wg nazw.

== Teacher ==

Klasa Teacher reprezentuje nauczyciela. Nauczyciel jest osobą. Nauczyciel ma
listę przedmiotów, które prowadzi.

Klasa Teacher ma konstruktor przyjmujący dwa parametry: imię, nazwisko.

Klasa Teacher nie ma konstruktora bezparametrowego.

Klasa Teacher ma metodę get_courses dającą niemodyfikowalny dostęp do jego
przedmiotów posortowanych wg nazw.

== PhDStudent ==

Klasa PhDStuden reprezentuje doktoranta. Doktorant jest jednocześnie studentem
i nauczycielem.

== College ==

Klasa College reprezentuje uczelnię. Klasa College zawiera kolekcje osób
i przedmiotów. Udostępnia publicznie następujące metody:

bool add_course(name, active = true);
// Dodaje przedmiot. Zwraca true, jeśli przedmiot został dodany, a false,
// jeśli istnieje już przedmiot o tej samej nazwie.

auto find_courses(pattern);
// Wyszukuje przedmioty o nazwie pasującej do podanego wzorca. Wzorzec może
// zawierać wieloznaczniki ? i *.

bool change_course_activeness(course, active);
// Zmienia status przedmiotu na aktywny lub nieaktywny zgodnie z podaną
// wartością. Zwraca true, jeśli przedmiot istnieje, a false w przeciwnym
// przypadku.

bool remove_course(course);
// Usuwa przedmiot (z kolekcji przedmiotów uczelni) i zmienia jego status
// na nieaktywny. Zwraca true, jeśli istnieje przedmiot, lub false w przeciwnym
// przypadku.

template <typename T>
bool add_person(name, surname, active = true);
// Dodaje studenta, doktoranta lub nauczyciela. Typ T może być tylko Student,
// PhDStudent lub Teacher. Trzeci parametr jest uwzględniany tylko wtedy, gdy
// T nie jest Teacher. Zwraca true, jeśli osoba o podanych imieniu i nazwisku
// została dodana, a false, jeśli już istnieje taka osoba.

bool change_student_activeness(student, active);
// Zmienia status studenta na aktywny lub nieaktywny zgodnie z podaną
// wartością. Zwraca true, jeśli student istnieje, a false w przeciwnym
// przypadku.

template <typename T>
auto find(name_pattern, surname_pattern);
// Wyszukuje osoby o imieniu i nazwisku pasującym do podanych wzorców.
// Typ T może być Person, Student, PhDStudent lub Teacher.
// Wzorce mogą zawierać wieloznaczniki ? i *.

template <typename T>
auto find(course);
// Wyszukuje studentów zapisanych na podany przedmiot (jeśli T = Student)
// lub nauczycieli prowadzących podany przedmiot (jeśli T = Teacher).

template <typename T>
bool assign_course(person, course);
// Przypisuje przedmiot studentowi lub nauczycielowi zależnie od tego, czy
// typ T jest to Student czy Teacher. Podnosi wyjątek przy próbie przypisania
// nieaktywnego przedmiotu lub nieaktywnemu studentowi. Podnosi również
// wyjątek, gdy dana osoba lub dany przedmiot nie istnieje.
// Funkcja zwraca true, jeśli przedmiot został przypisany studentowi lub
// nauczycielowi, a false, jeśli przedmiot był już wcześniej przypisany.

== Dodatkowe wymagania

Oprócz wyżej wymienionych metod, klasy mogą mieć inne metody niezbędne do
realizacji swoich funkcji.

Oprócz wyżej wymienionych klas, implementacja może zawierać inne klasy lub
struktury niezbędne do realizacji przedstawionych wymagań.

W wynikach metod find_courses i find przedmioty mają być psortowane wg ich nazw,
a osoby wg nazwiska i imienia. Wszelkie napisy zawierają tylko znaki ASCII,
a ich porównywanie odbywa się wg wartości kodów ASCII.

Tam, gdzie jest to możliwe i uzasadnione, należy opatrzyć metody kwalifikatorami
const i noexcept.

Przykład użycia znajduje się w pliku college_example.cc. Wynik jego działania
jest w pliku college_example.log.

Rozwiązanie musi zawierać plik nagłówkowy college.h. Oprócz tego rozwiązanie
może zawierać dodatkowe pliki nagłówkowe *.h oraz opcjonalnie pliki
źródłowe *.cc.

Rozwiązanie będzie kompilowane poleceniem

g++ -Wall -Wextra -O2 -std=c++20 *.cc

Pliki rozwiązania należy umieścić w repozytorium w katalogu

grupaN/zadanie6/ab123456+cd123456

lub

grupaN/zadanie6/ab123456+cd123456+ef123456

gdzie N jest numerem grupy, a ab123456, cd123456, ef123456 są identyfikatorami
członków zespołu umieszczającego to rozwiązanie. Katalog z rozwiązaniem nie
powinien zawierać innych plików. Nie wolno umieszczać w repozytorium plików
dużych, wykonywalnych, binarnych, tymczasowych (np. *.o) ani innych zbędnych.
