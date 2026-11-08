# Maker Projects

Colección de proyectos electrónicos pequeños de Nostec: microcontroladores, sensores, pantallas y conexión con el celular. Cada proyecto incluye su página web, su firmware y su documentación.

Página principal: https://nostec-dev.github.io/maker-projects/

Cada proyecto tiene su propia dirección, por ejemplo https://nostec-dev.github.io/maker-projects/01-ejercitador-fuerza/

## Proyectos

| Código | Proyecto | Categorías | Carpeta |
|---|---|---|---|
| P01 | Ejercitador de fuerza | Instrumentación · IoT y Bluetooth | [`01-ejercitador-fuerza`](01-ejercitador-fuerza/) |

## Cómo está organizado

```
maker-projects/
├── index.html                  Página principal con la lista de proyectos
├── README.md                   Este archivo
├── .nojekyll                   Hace que GitHub Pages publique los archivos tal cual
└── 01-ejercitador-fuerza/      Un proyecto = una carpeta
    ├── index.html              Página del proyecto
    ├── README.md               Qué es, conexiones y cómo usarlo
    ├── firmware/               Códigos para Arduino IDE (cada uno en su carpeta)
    └── img/                    Fotos y capturas
```

## Reglas para cada proyecto nuevo

1. **Código:** el siguiente número libre: P01, P02, P03…
2. **Carpeta:** el número con dos cifras y un nombre corto en minúsculas, separado con guiones: `02-monitor-co`, `03-red-malla-agricola`.
3. **Contenido:** siempre `index.html` y `README.md`. Agrega `firmware/` e `img/` cuando los tenga.
4. **Firmware:** cada `.ino` va dentro de una carpeta con su mismo nombre, como pide Arduino IDE.
5. **Foto:** una imagen horizontal 4:3 (1200 × 900) en `img/portada.jpg`; se muestra en la ficha.
6. **Categorías:** una o varias de Instrumentación, IoT y Bluetooth, Control, Potencia y Comunicaciones.
7. **Lista:** agrega la ficha del proyecto en `PROYECTOS`, dentro de `index.html`, y una fila en la tabla de arriba.
