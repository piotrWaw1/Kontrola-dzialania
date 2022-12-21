# Kontrola-działania

## Kompilacja

W folderze w którym znajduje się projekt wpisz polecenie: 

### `make init` - aby zainstalować wszystkie potrzebne elementy

### `make` - aby skompilować pliki

## Uruchomienie

### `./client liczba_hostow` - uruchamia program sprawdzający czy dana grupa urzytkowników pracuje (AN)

### `./server` - uruchamia program oczekujący na sprawdzenie pracy urzytkownika (AK)

### `./kill_server` - wyłącza aplikację nadzorującą (AN)

## Wymagania

### `ip_list` - plik zawierający listę ip hostów

## Pliki utworzone przez program

### `data.db` - zawiera dwie tabele *ip* i *history*

> *`ip`* zawiera listę adresów ip hostów zczytaną z pliku *ip_list*

> *`history`* zawiera informacje ot tym czy użytkownik i AK pracuje (id_address, date, is_working, is_responding)
