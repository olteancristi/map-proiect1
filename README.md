# Agendă de contacte

Proiect individual la disciplina Metode avansate de programare, anul universitar 2026-2027.

## Autor

- **Nume:** Oltean Cristian-Daniel
- **Grupa:** 2.1
- **Marca:** 15700
- **Tema:** 1 - Agenda de contacte

## Descriere

Aplicația implementează o agendă de contacte care permite gestionarea informațiilor despre persoane.
Datele contactelor pot fi adăugate, consultate și administrate prin intermediul serviciului implementat în C++.
Aplicația expune o interfață HTTP și răspunde în format JSON.


## Tehnologii

C++20 cu cpp-httplib si nlohmann/json

## Rulare

```
docker build -t map-proiect .
docker run -d -p 8080:8080 map-proiect
```

Aplicatia asculta pe portul 8080. Verificati:

```
curl http://localhost:8080/health
curl http://localhost:8080/version
```

## Testare

```
cmake -B build -DBUILD_TESTS=ON
cmake --build build -j
./build/tests
```

## Rutele implementate

| Ruta | Metoda | Descriere |
|---|---|---|
| `/health` | GET | Starea serviciului |
| `/version` | GET | Versiunea si commit-ul din care a fost construita imaginea |
| `/` | GET | Pagina de prezentare |
| `/reset` | POST | Goleste datele din memorie |
| [ruta temei] | [metoda] | [descriere] |


## Decizii de implementare

- Am ales C++20 ca limbaj de implementare, conform șablonului proiectului.
- Pentru implementarea rutelor HTTP folosesc biblioteca `cpp-httplib`, deoarece oferă o interfață simplă pentru definirea endpoint-urilor.
- Pentru lucrul cu date în format JSON folosesc biblioteca `nlohmann/json`, deoarece permite construirea și prelucrarea ușoară a obiectelor JSON în C++.