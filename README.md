# Maker Projects · Nostec

Portafolio de **Hugo Andrés Llanos Benítez** bajo la marca **Nostec**: desarrollo de hardware, sistemas embebidos y automatización.

Página: https://nostec-dev.github.io/maker-projects/

- **Proyecto en proceso:** [NST-FRC32](https://github.com/Nostec-Dev/NST-FRC32), tarjeta de control de una recicladora de botellas PET en filamento.
- **Desarrollos a medida:** proyectos por encargo, cada uno documentado con su hardware, firmware e interfaz. No se nombra al cliente.

## Desarrollos a medida

| Código | Proyecto | Aplicación | Documentación |
|---|---|---|---|
| P01 | Digiflex | Fisioterapia de mano | [Ver documentación](https://nostec-dev.github.io/maker-projects/01-ejercitador-fuerza/documentacion/) · [carpeta](01-ejercitador-fuerza/) |

## Cómo está organizado

```
maker-projects/
├── index.html                  Página principal del portafolio
├── README.md                   Este archivo
├── img/                        Imágenes de la página principal
├── _plantilla/                 Guía para documentar un desarrollo nuevo
├── .nojekyll                   Hace que GitHub Pages publique los archivos tal cual
└── 01-ejercitador-fuerza/      Un proyecto = una carpeta
    ├── index.html              Aplicación del proyecto (si tiene)
    ├── documentacion/          Página de documentación técnica
    ├── README.md               Resumen, conexiones y uso
    ├── firmware/               Códigos para Arduino IDE (cada uno en su carpeta)
    └── img/                    Fotos y capturas
```

## Proyecto nuevo

Sigue la guía de [`_plantilla/README.md`](_plantilla/README.md).
