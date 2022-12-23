# Kontrola-działania

## Kompilacja

W folderze w którym znajduje się projekt wpisz polecenie: 

### `make init` - aby zainstalować wszystkie potrzebne elementy.
> Zainstaluje:
> `gcc`, 
> `sqlite3`, 
> `libsqlite3-dev`

### `make` - aby skompilować pliki

## Uruchomienie

### `./client liczba_hostow` - uruchamia program sprawdzający czy dana grupa użytkowników pracuje (*AN*)

### `./server` - uruchamia program oczekujący na sprawdzenie pracy użytkownika (*AK*)

### `./kill_client` - wyłącza aplikację nadzorującą (*AN*)

## Wymagania

### `ip_list` - plik zawierający listę ip hostów

## Pliki utworzone przez program

### `data.db` - zawiera dwie tabele *ip* i *history*

> *`ip`* zawiera listę adresów ip hostów zczytaną z pliku *ip_list*

> *`history`* zawiera informacje o tym czy użytkownik i AK pracuje (id_address, date, is_working, is_responding)
