<p align="center">
  <img src="docs/images/libertyboard-logo.png" alt="LibertyBoard" width="160">
</p>

<h1 align="center">LibertyBoard</h1>

<p align="center"><b>Pizarra digital y lector PDF para escribir con lápiz, presentar y anotar sobre cualquier cosa.</b></p>

LibertyBoard es un programa para Windows pensado para quien trabaja con lápiz digital: abre un PDF y escribe encima, presenta con un puntero láser que se desvanece, o dibuja directamente sobre el escritorio mientras usas otros programas.

## Qué hace

- **Lector y anotador de PDF.** Abre un PDF y queda listo en la página 1, con miniaturas de páginas a la izquierda y un indicador de página editable ("1 / 200").
- **Puntero láser con desvanecimiento.** El trazo se borra solo después de un tiempo que tú eliges. También hay modo punto y efecto de brillo.
- **Paneles de herramientas claros.** Cada herramienta (lápiz, rotulador, borrador, láser, formas) muestra sus opciones en la barra superior: color, grosor, opacidad, tipo de trazo y más.
- **Formas.** Flecha, rectángulo, elipse y subrayado, con borde, relleno, línea punteada y puntas de flecha.
- **Resaltador de texto** y **notas** que muestran su contenido al pasar el cursor.
- **Modo escritorio.** Anota sobre cualquier programa abierto; la barra de herramientas se acopla al borde derecho, arriba o abajo.
- **Ventana normal con menús**, como cualquier programa de escritorio.

## Empezar

1. Descarga la última versión desde la pestaña **Actions** de este repositorio (artefacto `LibertyBoard-windows`).
2. Descomprime la carpeta y abre `LibertyBoard.exe`.

LibertyBoard guarda sus documentos y su configuración en su propia carpeta (`%LOCALAPPDATA%\LibertyBoard`). Empieza vacío: no lee ni copia nada de otros programas.

## Compilar desde el código

La versión de Windows se compila automáticamente con GitHub Actions (`.github/workflows/windows-build.yml`) usando Qt 6 y MSVC. Las notas de diseño y lo acordado para cada etapa están en [docs/LIBERTYBOARD.md](docs/LIBERTYBOARD.md).

## Licencia y origen

LibertyBoard es una **versión modificada de [OpenBoard](https://github.com/OpenBoard-org/OpenBoard)**, que a su vez deriva de Open-Sankoré y Uniboard. Se distribuye bajo la **Licencia Pública General de GNU, versión 3 (GPLv3)**, igual que OpenBoard, con la misma excepción de enlace para OpenSSL. Ver [LICENSE](LICENSE) y [COPYRIGHT](COPYRIGHT).

Los avisos de copyright originales se mantienen en los archivos de código. LibertyBoard no está afiliado ni respaldado por el proyecto OpenBoard ni por sus autores.
