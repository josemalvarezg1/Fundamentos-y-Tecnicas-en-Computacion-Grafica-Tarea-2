# Tarea 2 - Fundamentos y Técnicas en Computación Gráfica

## Tabla de contenido

* [Escena](#escena)
* [Modelos](#modelos)
* [Texturas normales](#texturas-normales)
* [Texturas de luz](#texturas-de-luz)
* [Herramientas](#herramientas)
* [Instalación y uso](#instalación-y-uso)
* [Creador](#creador)

# Escena 

La escena de esta tarea está compuesta por 21 objetos texturizados:
  - 1 plano que representa a un espejo.
  - 1 modelo refractor.
  - 3 planos que representan a las cercas de la escena.
  - 1 plano que representa al piso de la escena.
  - 14 modelos de personajes o modelos 3D.
  - 1 esfera que indica de dónde proviene la luz.

Para desplazarse en la escena se pueden utilizar las teclas W, A, S, D y el ratón para mover la cámara. Si por comodidad se desea desactivar la cámara del ratón, se debe presionar la tecla T. Al realizar esto, se esconderá el menú de AntTweakBar y el mapa de profundidad en caso de que se esté mostrando.

El modelo de interpolación general es Phong, y el modelo de iluminación general es Blinn-Phong.

# Modelos

Cada objeto puede ser seleccionado con el click (a excepción del plano que representa al piso de la escena). 

Al seleccionar cualquier objeto, se le puede aplicar libremente transformaciones como rotación, traslación y escalamiento. Este último se puede ajustar con la rueda del ratón. A su vez, se les pueden cambiar sus colores ambiental, difuso y especular, activar o desactivar sus sombras, ajustar el factor bias de sus sombras (a excepción de que estén activadas las sombras puntuales), y activar o desactivar su textura asociada.  

Al seleccionar la esfera de luz se puede cambiar la ubicación de la luz, encenderla y apagarla, sus colores ambiental, difuso y especular, su tipo, cambiar el tipo de sombras, y activar o desactivar el mapa de profundidad. Para cada tipo de luz se mostrarán sus respectivos parámetros que pueden ser editados.

Cabe destacar que al modelo refractor y al modelo reflector no le afecta la luz.

# Texturas normales

Los modelos de las cercas, Green Arrow y Flash, poseen texturas normales. Estas últimas pueden ser desactivadas en cualquier momento que se seleccione a su modelo asociado. 

# Texturas de luz

El modelo del cubo y el cuadro de La Mona Lisa poseen texturas de luz (light maps). Estas últimas pueden ser desactivadas en cualquier momento que se seleccione a su modelo asociado. 

![alt tag](http://i.imgur.com/hxVWtFd.jpg)

# Herramientas

En la presente tarea se utilizaron las siguientes herramientas con sus respectivas versiones:

| Herramienta                         	 | Versión   													   |                            
|----------------------------------------|-----------------------------------------------------------------|
| Microsoft Visual Studio        	 	 | 2015      													   |


# Instalación y uso
Se deberá descargar el siguiente repositorio o clonarlo con el comando:

git clone https://github.com/josemalvarezg1/Fundamentos-y-Tecnicas-en-Computacion-Grafica-Tarea-2.git

Se puede abrir el archivo .sln en Visual Studio o ejecutar directamente el proyecto desde la carpeta bin/.

![alt tag](http://i.imgur.com/s3oCmmS.jpg)


# Creador

**José Manuel Alvarez García - CI 25038805**