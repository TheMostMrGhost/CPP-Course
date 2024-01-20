Biblioteka standardowa języka C++ udostępnia implementacje wielu struktur
danych, takich jak pair, vector, unordered_set, set, string, unordered_map,
map, queue itp., a także implementacje podstawowych algorytmów, np. sort,
lower_bound, upper_bound, max_element itd. Celem pierwszego zadania
zaliczeniowego jest przećwiczenie korzystania z tej biblioteki.

Napisać program weryfikujący, czy opłacono parkowanie pojazdu. Płatne parkowanie
obowiązuje codziennie od godziny 8.00 do godziny 20.00. Opłatę za parkowanie
trzeba uiścić minimum za 10 minut. Maksymalny czas, na jaki można uiścić opłatę
za parkowanie, to 11 godzin i 59 minut, czyli można na przykład zapłacić za
parkowanie od godziny 8.00 do godziny 19.59 tego samego dnia albo na przykład od
godziny 19.00 do godziny 18.59 dnia następnego.

Program czyta dane ze standardowego wejścia. Linie z danymi wejściowymi mają
jeden z następujących dwóch formatów.

1. Wprowadzenie informacji o uiszczeniu opłaty:

<numer rejestracyjny pojazdu> <czas rozpoczęcia parkowania> <czas zakończenia parkowania>

2. Sprawdzenie, czy uiszczono opłatę:

<numer rejestracyjny pojazdu> <bieżący czas>

Numer rejestracyjny pojazdu składa się z 3 do 11 znaków będących wielką literą
alfabetu angielskiego lub cyfrą, przy czym pierwszy znak musi być literą.
Pola opisujące czas składają się z godziny i minuty rozdzielonych kropką.
Godzina może być wyrażona jedną lub dwoma cyframi (wiodące zero jest
opcjonalne). Minuta jest wyrażona zawsze dwoma cyframi.

Poprawne wartości czasu zaczynają się od godziny 8.00 i kończą się o godzinie
20.00. Wartości pól czas rozpoczęcia parkowania i bieżący czas w kolejnych
liniach danych wejściowych uporządkowane są słabo monotonicznie.

Pola w linii danych oddzielone są białymi znakami. Na początku i na końcu linii
też mogą pojawić się białe znaki.

Program potwierdza wprowadzenie informacji o uiszczeniu opłaty, wypisując na
standardowe wyjście komunikat:

OK L

Program potwierdza uiszczenie opłaty, wypisując na standardowe wyjście
komunikat:

YES L

Program potwierdza brak uiszczenie opłaty, wypisując na standardowe wyjście
komunikat:

NO L

Program na bieżąco sprawdza, czy dane wejściowe nie zawierają błędów. Dla każdej
błędnej linii program wypisuje na standardowe wyjście diagnostyczne komunikat:

ERROR L

W powyższych komunikatach L oznacza numer linii. Linie są numerowane od 1,
włączając w to linie zawierające błędy.

Program powinien usuwać z pamięci niepotrzebne już informacje.

Program powinien kończyć się kodem 0.

Przykład użycia znajduje się w plikach test_example.in, test_example.out,
test_example.err.

Oczekiwane rozwiązanie nie powinno zawierać definicji własnych struktur i klas,
a przynajmniej takich, które zawierają dane. Zamiast tego należy intensywnie
korzystać z kontenerów i algorytmów dostarczanych przez standardową bibliotekę
języka C++. Obsługę wejścia i wyjścia należy zrealizować za pomocą strumieni.

Rozwiązanie należy umieścić w pliku parking.cc, który należy wstawić do
repozytorium SVN w katalogu

grupaN/zadanie1/ab123456+cd123456

lub

grupaN/zadanie1/ab123456+cd123456+ef123456

gdzie N jest numerem grupy, a ab123456, cd123456, ef123456 są identyfikatorami
członków zespołu umieszczającego to rozwiązanie. Katalog z rozwiązaniem nie
powinien zawierać innych plików. Nie wolno umieszczać w repozytorium plików
dużych, binarnych, tymczasowych (np. *.o) ani innych zbędnych.

Rozwiązanie będzie kompilowane na maszynie students poleceniem

g++ -Wall -Wextra -O2 -std=c++20 parking.cc -o parking
