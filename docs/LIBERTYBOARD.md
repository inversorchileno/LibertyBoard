# LibertyBoard

LibertyBoard es una pizarra digital y lector PDF con puntero láser estilo Drawboard PDF y una interfaz tradicional, en ventana. Es una versión modificada de OpenBoard (GPLv3). Este archivo reúne lo acordado con el dueño del proyecto, para que cualquier sesión de trabajo nueva pueda retomarlo.

## Etapas

1. **Compilación para Windows, láser nuevo y orden de las barras.** *(hecha)*
2. **Paneles de opciones estilo Drawboard y formas nuevas.** *(hecha)*
3. **Rediseño a ventana normal con menús (modo pizarra y modo documentos).** *(hecha)*
4. **Identidad propia: nombre, logo, carpeta de datos y documentación.** *(hecha)*
5. **Menús, guardar, actualización, zoom, notas de colores, idiomas y triángulos de opciones.** *(hecha, compilaciones 9 a 13)*

## 1. Barras de herramientas

Ambas barras van en gris oscuro sólido, sin transparencia.

**Modo escritorio**, en orden: Modo pizarra, Lápiz, Borrador, Rotulador, Láser, Selección, Flecha, Rectángulo, Elipse, Subrayar, Texto. Se quitan el teclado virtual, la captura de área y la captura de pantalla.

**Modo pizarra**, en orden: Lápiz, Borrador, Rotulador, Láser, Selección, Interactuar, Mano, Zoom +, Zoom −, Línea, Texto, Flecha, Rectángulo, Elipse, Subrayar. Se quitan la captura, el teclado virtual y el imán de la cuadrícula.

## 2. Opciones de cada herramienta (estilo Drawboard)

En modo escritorio son paneles emergentes; en modo pizarra, un panel fijo a la derecha. Los paneles son fijos: sin candado, papelera ni "…".

- **Lápiz:** tipo de trazo Básico (ancho fijo), Pro (ancho variable) o Caligrafía (ancho según dirección); grosor 1–50; 9 colores más un selector de color; opacidad.
- **Goma:** Whole (borra el trazo completo) o Partial (borra por donde pasa); tamaño 1–50.
- **Rotulador:** 9 colores de resaltador más selector; grosor 20–100; opacidad.
- **Rectángulo y elipse:** borde y relleno con colores independientes; grosor; tipo de línea (continua, punteada, guiones…); borde recto u ondulado tipo nube; opacidad.
- **Flecha:** colores; grosor; tipo de línea; puntas de inicio y fin configurables (ninguna, cuadrado, círculo, rombo, flecha abierta, flecha cerrada…); opacidad.
- **Subrayar:** línea recta horizontal, 100 % opaca, trazada bajo la palabra; colores y grosor.
- **Texto:** fuente, tamaño, color, negrita y cursiva.
- **Láser:** modo Trail (estela) o Point (punto); grosor 0,5–64; colores; efecto Standard o White burn (centro blanco con halo); opacidad; tiempo de espera y duración del desvanecimiento de 100 % a 0 %.
- Los últimos ajustes de cada herramienta se recuerdan al volver a abrir el programa.

## 3. Modo pizarra en ventana normal

- Ventana normal (maximizable), con la barra de tareas de Windows visible.
- Barra de herramientas a la izquierda, panel de opciones fijo a la derecha.
- Menú superior tradicional con Pizarra, Web, Documentos, Escritorio y Configuración.
- Fila de íconos bajo el menú: Deshacer y Repetir; Página nueva, Anterior y Siguiente; Fondos y Borrar.
- Sin las pestañas laterales de miniaturas ni de biblioteca.
- En modo escritorio desaparece todo y queda solo la barra flotante.
- El modo Web recibe el mismo tratamiento: ventana normal y menú superior.

## 4. Modo Documentos en ventana normal

- Mismo menú superior que la pizarra.
- Menú nuevo "Documento" con todas las acciones de la barra inferior izquierda: Nuevo documento, Nueva carpeta, Importar, Exportar, Cambiar nombre, Favorito, Duplicar, Papelera, Abrir en la pizarra, Añadir al documento y Añadir…, con atajos de teclado e íconos.
- Fila de íconos con las acciones frecuentes: Nuevo documento, Importar, Exportar, Abrir en la pizarra y Papelera.
- Se elimina el teclado virtual flotante en todo el programa.

## Cómo quedó la Etapa 2

- Las opciones de cada herramienta se abren en una ventanita estilo Drawboard:
  - en modo escritorio, con la flechita del botón o manteniendo presionado el botón;
  - en modo pizarra, con doble clic sobre la herramienta. En la Etapa 3 pasan al panel fijo de la derecha.
- Los ajustes se guardan al cerrar el programa.
- La flecha, el rectángulo, la elipse y el subrayado se guardan como trazos (polígonos), igual que el lápiz: se pueden mover, borrar y deshacer, y cualquier versión de OpenBoard abre los documentos. Una vez dibujados no se editan como forma (no se cambia su relleno después).
- Mantener Shift dibuja cuadrados y círculos, y flechas en ángulos de 15 grados.
- Los trazos Pro y Caligrafía dependen del lápiz: con mouse el Pro se ve como el Básico.
- El color del lápiz y del rotulador reemplaza el color seleccionado de la barra inferior de la pizarra, y el grosor reemplaza el grosor seleccionado (fino, medio o grueso).

## Cómo quedó la Etapa 3

- El programa abre en una ventana maximizada, con la barra de tareas de Windows visible. El modo escritorio sigue cubriendo toda la pantalla.
- Menú superior: **Ver** (Pizarra, Web, Documentos, Mostrar escritorio), **Documento** (solo en el modo Documentos) y **LibertyBoard** (tamaño de página, copiar y pegar, tutorial, preferencias, podcast, salir).
- Modo pizarra: barra de herramientas a la izquierda, fila de íconos bajo el menú (Deshacer y Repetir; Páginas, Anterior y Siguiente; Fondos y Borrar) y panel de opciones fijo a la derecha, que cambia con la herramienta elegida. El doble clic sobre una herramienta sigue abriendo sus opciones junto al botón.
- Modo Documentos: fila con Nuevo documento, Importar, Exportar, Abrir en la pizarra y Papelera; el resto en el menú Documento, con atajos: Ctrl+N, Ctrl+Mayús+N, Ctrl+I, F2, Ctrl+Mayús+D, Supr y Ctrl+O.
- Sin pestañas laterales (miniaturas y biblioteca) ni teclado virtual.
- La ventana y la barra vertical vienen así por defecto en la configuración de LibertyBoard (`RunInWindow=true`, `ToolBarOrientationVertical=true`).

## Ajustes después de la Etapa 3

- Abre en modo escritorio, con la barra desplegada.
- La barra de herramientas de la pizarra va por defecto a la derecha. Al soltarla junto al borde de arriba o de abajo se vuelve horizontal; junto a la izquierda o la derecha, vertical. LibertyBoard recuerda el borde.
- La fila de íconos está arriba (con su propio ajuste, independiente de OpenBoard) y muestra las opciones de la herramienta elegida: ya no hay panel a la derecha. Páginas, Anterior, el contador de páginas, Siguiente, Fondos y Borrar quedan a la derecha.
- En el menú, a la derecha: botón **Escritorio** y un triángulo que oculta o muestra la fila de íconos.
- Contador de páginas «1 / 200»: se escribe un número y Enter lleva a esa página.
- Barra lateral izquierda con las miniaturas de las páginas, que se oculta o muestra con un triángulo.
- Al importar un archivo (por ejemplo un PDF) se abre de inmediato en la pizarra, en la hoja 1.
- **Resaltar:** una banda horizontal semitransparente que se arrastra sobre una línea de texto, con colores, alto y opacidad. No detecta el texto del PDF: se arrastra a lo largo de la línea.
- **Nota:** un clic pone un marcador tipo cinta con un texto. Al pasar el puntero se ve el texto; al hacer clic (con la herramienta Nota, Selección o Interactuar) se puede actualizar, borrar o cambiar de color. Se guarda en el documento como `ub:note` con `ub:color`; OpenBoard normal no lo muestra.

## Compilación y publicación

- GitHub Actions compila LibertyBoard para Windows en cada push a `master` (`.github/workflows/windows-build.yml`), salvo cuando el push solo cambia documentación (`docs/**`, `*.md`). Qt 6.8.3 MSVC, qmake (`OpenBoard.pro` y los `.pri`), librerías de `OpenBoard-ThirdParty`, QuaZip compilado en el mismo flujo.
- Cada compilación de `master` se publica como release `build-<número de ejecución>` con `LibertyBoard-windows.zip`, y ese número queda compilado en el programa (`LIBERTYBOARD_BUILD`). El menú **Acerca → Actualización** (`src/core/UBGitHubUpdater`) compara ese número con el último release, descarga el zip y lo instala con un script de PowerShell que espera a que LibertyBoard se cierre, copia los archivos y lo vuelve a abrir.
- Las traducciones se generan con `lrelease` **antes** de `qmake`; si no, los `.qm` no se instalan y el programa queda en inglés. La compilación falla si falta `OpenBoard_es.qm`.
- Los archivos nuevos se registran en **los dos** sistemas: qmake (`src/*/*.pri`, para Windows) y CMake (`src/*/CMakeLists.txt`, para las pruebas en Linux). Los recursos nuevos van en `resources/OpenBoard.qrc`.
- Una compilación tarda unos 15 a 20 minutos. Se sigue con `gh api repos/noteimportaaaa/LibertyBoard/actions/runs`.

## Identidad de LibertyBoard

- **Nombre.** El programa se llama LibertyBoard en todo lo visible: ventana, menús, mensajes, ayudas de inicio, traducciones e información del ejecutable de Windows (`LibertyBoard.exe`). El nombre interno de algunos archivos del código (`OpenBoard.pro`, `OpenBoard.qrc`, `OpenBoard_es.ts`) se mantiene para no romper la compilación ni facilitar traer mejoras de OpenBoard.
- **Logo.** Tarjeta blanca con una tableta naranja con firma azul, un lápiz azul escribiendo y alas azules a los lados. El original vectorial está en `docs/images/libertyboard-logo.svg`; los iconos del programa (`resources/images/OpenBoard.png`, `bigOpenBoard.png`, `ch.openboard.OpenBoard.svg`, `resources/win/LibertyBoard.ico`) se generan desde él.
- **Carpeta propia.** El nombre de aplicación y de organización es LibertyBoard, así que los documentos y la configuración del usuario quedan en `%LOCALAPPDATA%\LibertyBoard` y el archivo de usuario se llama `LibertyBoardUser.config`. No se lee ni se copia nada de una instalación de OpenBoard: quien instala LibertyBoard parte de cero. La configuración de fábrica está en `resources/etc/LibertyBoard.config` y el estilo en `LibertyBoard.css`.
- **Sin actualizaciones de OpenBoard.** La búsqueda de actualizaciones está apagada y los enlaces de inicio y tutorial apuntan al repositorio de LibertyBoard.
- **Créditos.** La pestaña Acerca de muestra primero LibertyBoard y luego indica que es una versión modificada de OpenBoard, con sus créditos y la licencia GPLv3. Los encabezados de copyright de los archivos de código se mantienen tal cual, como exige la licencia.

## Cómo quedó la Etapa 5

- **Menús** (4): **Inicio** (Abrir/Importar, Guardar, Preferencias, Multipantalla, Podcast, Cerrar documento, Salir), **Modo** (Pizarra, Web, Escritorio), **Edición** (Tamaño de página, Cortar, Copiar, Pegar, Ocultar LibertyBoard, Dormir) y **Acerca** (Tutorial, Consejos, LibertyBoard en GitHub, Actualización). Código en `src/gui/UBMainMenu.cpp`.
- **Guardar** (Ctrl+S, `src/board/UBBoardSaver`): documento que viene de un PDF → PDF con las anotaciones; pizarra en blanco → PNG o JPEG de la página.
- **Cerrar documento** (Ctrl+F4, icono X roja): pregunta si guardar (PDF o imagen) y deja una pizarra limpia como al iniciar; el documento sigue en Abrir/Importar.
- **Zoom:** recuadro fijo abajo a la derecha de la pizarra con −, % editable y + (`src/gui/UBZoomPalette`). Ya no hay zoom en la barra de herramientas.
- **Rueda del mouse en PDF:** al pasar el final de la página va a la siguiente desde arriba, y al pasar el inicio a la anterior desde abajo, con el mismo zoom (`UBBoardView::turnPageWithWheel`). No es una vista continua.
- **Orden de herramientas** (escritorio y pizarra): Lápiz, Borrador, Rotulador, Láser, Resaltar, Interactuar, Nota, Subrayar, Texto, [Línea, solo pizarra], Flecha, Rectángulo, Elipse; en pizarra al final las dos manos (Interactuar con contenidos y Mano); en escritorio al final la **captura de pantalla** (cámara), que guarda pantalla + rayas en `Imágenes\LibertyBoard`.
- **Triángulos de opciones:** todos negros con borde blanco (`src/gui/UBToolOptionsArrows`). Apuntan hacia donde se abren las opciones: barra a la derecha → apunta a la izquierda, abajo-izquierda del icono; a la izquierda → a la derecha, abajo-derecha; arriba → hacia abajo, abajo-centro; abajo → hacia arriba, arriba-centro. En escritorio la barra es vertical y vale la mitad de la pantalla donde esté. La ventana de opciones se abre hacia ese mismo lado.
- **Opciones:** iconos estilo Drawboard en Básico/Pro/Caligrafía, Borde/Relleno, Estela/Punto y Estándar/Brillo blanco; la vista previa del lápiz cambia según el tipo de trazo; el láser muestra los títulos y valores encima de sus barras; la sección de color se llama "Color".
- **Notas de colores:** 7 colores (amarillo, rojo, naranjo, verde, azul, morado, gris), elegibles en la barra de la herramienta Nota y en la ventana de la nota. Las notas van en la capa de los dibujos, sobre el PDF, y salen en el PDF guardado.
- **Idioma:** Preferencias → Mostrar → Idioma (primera opción): idioma del sistema o uno de los 33. Se aplica al reiniciar. Todo texto nuevo de LibertyBoard se traduce a los 33 idiomas en un commit `i18n:` aparte.
- **Preferencias** con fondo claro y letras negras; LibertyBoard fuerza el tema claro (Qt 6.8) porque el modo oscuro de Windows dejaba texto oscuro sobre fondo oscuro.
- **Multipantalla** apagada por defecto.
- **Portable:** el dueño usa el zip, sin instalador. Para que aparezca en programas que listan aplicaciones (como Stream Dock), se pone un acceso directo en `shell:common programs`.

## Forma de trabajo

- El dueño es **Luis** (Chile), analista financiero, sin experiencia en programación. Habla en español; se le responde en español, claro y sin jerga. Usa Windows 11, pantalla ancha (unos 2540 px) y lápiz digital.
- Pide cambios en tandas, a menudo con capturas marcadas en verde. Si dice "no compiles aún", se hace commit sin push a `master` (o push a otra rama).
- Antes de cada push: compilar en Linux y probar lo visible en la aplicación real, con capturas. Al terminar, decirle qué se probó y qué no, y qué debe revisar él en Windows.
- Push a `master` = nueva versión para él. Se le avisa del número de compilación y que actualice con **Acerca → Actualización**.
- Commits en inglés con el formato `tipo: resumen` y un cuerpo que explica qué se hizo y en qué se probó (ver `CODING_STYLE.md`). Nunca usar sus documentos reales para pruebas.
- Respetar la GPLv3: se mantienen los encabezados de copyright y la mención "versión modificada de OpenBoard".

## Pruebas en Linux

- Compilar con CMake + Ninja (Qt 6 del sistema) e instalar con `ninja install` (queda como `/usr/local/bin/openboard`).
- Pantalla virtual: `Xvfb :99 -screen 0 2540x1000x24 -dpi 96` + `openbox`; abrir con `HOME=<carpeta de prueba> LANG=es_ES.UTF-8 LANGUAGE=es QTWEBENGINE_DISABLE_SANDBOX=1 LIBGL_ALWAYS_SOFTWARE=1`. Clics con `xdotool`, capturas con `import -window root`.
- Para abrir un PDF de prueba: ejecutar un segundo `openboard <archivo.pdf>`, que lo pasa al programa ya abierto. No reutilizar el mismo nombre de PDF (pregunta si reemplazar).

## Ideas pendientes

- **Instalador** (Inno Setup, hay un script de OpenBoard en `release_scripts/windows`): por ahora el dueño prefiere el portable.
- **Vista continua** de las páginas del PDF (todas una debajo de otra), en vez de pasar página a página.
- Importar un PDF con el mismo nombre que uno ya importado pregunta si reemplazar; para un lector de PDF debería abrirlo sin preguntar.
- Se vio una vez que el programa se cerraba al abrir un segundo PDF mientras aparecía esa pregunta de reemplazo (viene de OpenBoard); revisar.
- La ventana de bienvenida (consejos) se abre al inicio hasta que se desmarca.
