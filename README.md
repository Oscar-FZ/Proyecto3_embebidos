# Proyecto3_embebidos

# Sistema de Recetas Médicas

Este es un sistema simple de recetas médicas desarrollado en Python utilizando la biblioteca `tkinter` para la interfaz gráfica. El sistema permite ingresar el nombre de un paciente, agregar recetas, consultar recetas existentes y enviar la receta a través de una conexión TCP/IP a una dirección específica.

## Requisitos

- Python 3 instalado.
- Biblioteca `tkinter` incluida en la instalación de Python.

## Instrucciones de Uso

1. Ejecute el archivo Python (`inter.py`).
2. Ingrese el nombre del paciente en la ventana principal y haga clic en el botón "Aceptar".
3. Se abrirá una nueva ventana con el menú principal que le permitirá:
   - Agregar una receta.
   - Consultar recetas existentes para el paciente.
   - Salir del sistema.

### Agregar Receta

1. Haga clic en el botón "Agregar Receta" en el menú principal.
2. Seleccione los medicamentos de la lista proporcionada.
3. Los medicamentos seleccionados se mostrarán en la parte inferior de la ventana.
4. Haga clic en el botón "Enviar y Guardar Receta" para guardar la receta y enviarla a través de una conexión TCP/IP.

### Consultar Recetas Existentes

1. Haga clic en el botón "Consultar Recetas Existentes" en el menú principal.
2. Se mostrarán las recetas existentes para el paciente en una nueva ventana.

### Salir

1. Haga clic en el botón "Salir" en el menú principal para cerrar la aplicación.

## Notas Importantes

- El sistema guarda las recetas en un archivo llamado `recetas.txt`.
- La conexión TCP/IP está configurada para enviar información a la dirección IP `192.168.173.49` y al puerto `10000`.
- Se utiliza un tiempo de espera de 3 segundos (`time.sleep(3)`) antes de cerrar la conexión.

**¡Importante!**
Este sistema es un ejemplo educativo y podría requerir adaptaciones para su uso en un entorno real. 
