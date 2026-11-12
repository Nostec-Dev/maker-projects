# Plantilla para documentar un desarrollo a medida

Cada encargo se documenta igual. El ejemplo completo es Digiflex: [`01-ejercitador-fuerza/documentacion/index.html`](../01-ejercitador-fuerza/documentacion/index.html). Para un proyecto nuevo, copia esa página y cambia el contenido de cada sección.

## 1. Datos del proyecto

- **Código:** el siguiente número libre: P02, P03…
- **Carpeta:** número de dos cifras y nombre corto en minúsculas con guiones, por ejemplo `02-aforo-parqueaderos`.
- **Nombre** del producto y **aplicación** (por ejemplo "Fisioterapia de mano"). No nombres al cliente ni a la institución.
- **Fecha** (mes y año) y **plataforma** (por ejemplo "ESP32 · BLE").
- **Categorías:** una o varias de Instrumentación, IoT y Bluetooth, Control, Potencia, Comunicaciones.

## 2. Secciones de la página de documentación

| # | Sección | Qué va |
|---|---|---|
| 01 | Requerimiento | Qué se pidió, en 3 o 4 puntos: medir, controlar, mostrar, registrar… |
| 02 | Solución | Cómo se resolvió y diagrama de bloques (medición y conectividad) |
| 03 | Hardware | Tabla de componentes (componente, función, detalle) y tabla de conexiones |
| 04 | Firmware | Entorno y librerías, funciones clave y tabla de programas |
| 05 | Interfaz | Pantalla, aplicación o página web, si tiene |
| 06 | Pruebas y resultados | 4 cifras (rango, resolución, calibración, frecuencia…), problemas resueltos y limitaciones |
| 07 | Galería | 3 imágenes: equipo completo, interfaz y un resultado (informe, gráfica, montaje) |
| 08 | Archivos | Enlaces a la aplicación, el firmware, el README y la carpeta |

## 3. Archivos

```
NN-nombre/
├── documentacion/index.html    Copia de la página de Digiflex con el contenido nuevo
├── index.html                  Aplicación web (solo si el proyecto tiene)
├── README.md                   Resumen, enlace a la documentación, conexiones y uso
├── firmware/<programa>/<programa>.ino
└── img/
    ├── portada.jpg             Horizontal 4:3, 1200 × 900
    └── …                       Capturas y fotos de la galería
```

## 4. Publicar

1. Agrega la ficha en la lista `PROYECTOS` de `index.html` (raíz), con `doc: "documentacion/"` y, si tiene aplicación, `app: "Nombre del botón"`.
2. Agrega una fila en la tabla del `README.md` de la raíz.
3. Sube los cambios a `main`. GitHub Pages publica en uno o dos minutos.
