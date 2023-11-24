import tkinter as tk
from tkinter import messagebox, ttk
import socket
import struct
import time


class InterfazRecetas:
    def __init__(self, master):
        self.master = master
        self.master.title("Sistema de Recetas Médicas")
        self.master.geometry("800x600")
        self.master.configure(bg="light sky blue")

        self.nombre_paciente_label = tk.Label(master, text="Nombre del Paciente:", font=("Arial", 16))
        self.nombre_paciente_label.pack(pady=20)

        self.nombre_paciente_entry = tk.Entry(master, font=("Arial", 16))
        self.nombre_paciente_entry.pack(pady=10, ipadx=30, ipady=10)
        self.nombre_paciente_entry.config(width=50)

        self.aceptar_button = tk.Button(master, text="Aceptar", command=self.cerrar_ventana, font=("Arial", 16))
        self.aceptar_button.pack(pady=40, ipadx=80, ipady=20)

        self.receta_window = None  # Agrega esta línea

    def cerrar_ventana(self):
        self.nombre_paciente = self.nombre_paciente_entry.get()

        if not self.nombre_paciente:
            messagebox.showerror("Error", "Por favor, ingrese el nombre del paciente.")
            return

        self.master.withdraw()  # Oculta la ventana principal
        self.menu_principal()

    def menu_principal(self):
        menu = tk.Toplevel(self.master)
        menu.title(f"Bienvenido, recetas del paciente {self.nombre_paciente}")
        menu.geometry("800x600")
        menu.configure(bg="light sky blue")

        # Botón para agregar receta
        agregar_receta_button = tk.Button(menu, text="Agregar Receta", command=self.crear_receta, font=("Arial", 26))
        agregar_receta_button.pack(pady=30)
        agregar_receta_button.config(width=25, height=3)

        consultar_recetas_button = tk.Button(menu, text="Consultar Recetas Existentes", command=self.consultar_recetas, font=("Arial", 26))
        consultar_recetas_button.pack(pady=30)
        consultar_recetas_button.config(width=25, height=3)

        salir_button = tk.Button(menu, text="Salir", command=self.master.destroy, font=("Arial", 26))
        salir_button.pack(pady=30)
        salir_button.config(width=25, height=3)

    def consultar_recetas(self):
        try:
            with open("recetas.txt", "r") as file:
                recetas = [line.strip() for line in file.readlines() if self.nombre_paciente in line]
        except FileNotFoundError:
            recetas = []

        if not recetas:
            messagebox.showinfo("Aviso", "No hay recetas para este paciente.")
            return

        consulta_window = tk.Toplevel(self.master)
        consulta_window.title(f"Recetas Existentes para {self.nombre_paciente}")
        consulta_window.geometry("800x600")
        consulta_window.configure(bg="light sky blue")

        lista_recetas = tk.Listbox(consulta_window, width=60, height=15, font=("Arial", 14))
        for receta in recetas:
            lista_recetas.insert("end", receta)

        lista_recetas.pack(pady=10)


    def crear_receta(self):
        receta_window = tk.Toplevel(self.master)
        receta_window.title(f"Receta para {self.nombre_paciente}")
        receta_window.geometry("800x600")
        receta_window.configure(bg="light sky blue")

        # Lista de medicamentos permitidos
        medicamentos_permitidos = [
            "Acetaminofen", "Ibuprofeno", "Venlafaxina", "Aspirina", "Paracetamol",
            "Lobastatina", "Enalapril", "Lansoprazol", "Lukaler", "Ansiolit",
            "Metformina", "Amlodipino", "Viajesan", "Amitriptilina", "Tianina", "Decatylen"
        ]

        # Botones de medicamentos
        rows = 4
        cols = 4
        medicamentos_buttons = []

        for i in range(rows):
            row_buttons = []
            for j in range(cols):
                index = i * cols + j
                if index < len(medicamentos_permitidos):
                    medicamento = medicamentos_permitidos[index]
                    boton_medicamento = tk.Button(receta_window, text=medicamento, command=lambda m=medicamento: self.agregar_medicamento(m), font=("Arial", 14))
                    boton_medicamento.grid(row=i, column=j, padx=10, pady=10)
                    boton_medicamento.config(width=15, height=2)
                    row_buttons.append(boton_medicamento)
            medicamentos_buttons.append(row_buttons)

        # Etiqueta para mostrar medicamentos seleccionados
        medicamentos_label = tk.Label(receta_window, text="Medicamentos en la receta:", font=("Arial", 14))
        medicamentos_label.grid(row=rows, columnspan=cols, pady=10, ipadx=10, ipady=10)

        self.medicamentos_entry = tk.Entry(receta_window, width=60, font=("Arial", 14), state="readonly")
        self.medicamentos_entry.grid(row=rows + 1, columnspan=cols, pady=10, ipadx=10, ipady=10)

        # Botón para borrar el último medicamento ingresado
        borrar_ultimo_button = tk.Button(receta_window, text="Borrar Último Medicamento", command=self.borrar_ultimo_medicamento, font=("Arial", 14))
        borrar_ultimo_button.grid(row=rows + 2, columnspan=cols, pady=10, ipadx=10, ipady=10)

        # Botón para guardar la receta
        guardar_receta_button = tk.Button(receta_window, text="Eviar y Guardar Receta", command=self.guardar_receta, font=("Arial", 14))
        guardar_receta_button.grid(row=rows + 3, columnspan=cols, pady=10, ipadx=10, ipady=10)

    def agregar_medicamento(self, medicamento):
        # Agrega el medicamento al Entry de la receta
        current_medicamentos = self.medicamentos_entry.get()
        if current_medicamentos:
            current_medicamentos += f", {medicamento}"
        else:
            current_medicamentos = medicamento

        self.medicamentos_entry.config(state="normal")
        self.medicamentos_entry.delete(0, "end")
        self.medicamentos_entry.insert(0, current_medicamentos)
        self.medicamentos_entry.config(state="readonly")

    def borrar_ultimo_medicamento(self):
        # Borra el último medicamento ingresado de la receta
        current_medicamentos = self.medicamentos_entry.get()
        if current_medicamentos:
            medicamentos_list = current_medicamentos.split(", ")
            medicamentos_list.pop()
            updated_medicamentos = " ,".join(medicamentos_list)

            self.medicamentos_entry.config(state="normal")
            self.medicamentos_entry.delete(0, "end")
            self.medicamentos_entry.insert(0, updated_medicamentos)
            self.medicamentos_entry.config(state="readonly")

    def guardar_receta(self):
        medicamentos = self.medicamentos_entry.get()
        medicamento=[]
        envio=[]
        
        if not medicamentos:
            messagebox.showerror("Error", "La receta está vacía. Por favor, agregue al menos un medicamento.")
            return
        with open("recetas.txt", "a") as file:
            file.write( f"{self.nombre_paciente}:{medicamentos}\n")
        
        medicamento=[]
        envio=[0, 0]
        lista_medicamentos = medicamentos.split(', ')
        print (lista_medicamentos)
        
        for medicamento in lista_medicamentos:
            if medicamento == "Acetaminofen":
                envio.extend([1, 1])
            elif medicamento == "Ibuprofeno":
                envio.extend([1, 2])
            elif medicamento == "Venlafaxina":
                envio.extend([1, 3])
            elif medicamento == "Aspirina":
                envio.extend([1, 4])
            elif medicamento == "Paracetamol":
                envio.extend([2, 1])
            elif medicamento == "Lobastatina":
                envio.extend([2, 2])
            elif medicamento == "Enalapril":
                envio.extend([2, 3])
            elif medicamento == "Lansoprazol":
                envio.extend([2, 4])
            elif medicamento == "Lukaler":
                envio.extend([3, 1])
            elif medicamento == "Ansiolit":
                envio.extend([3, 2])
            elif medicamento == "Metformina":
                envio.extend([3, 3])
            elif medicamento == "Amlodipino":
                envio.extend([3, 4])
            elif medicamento == "Viajesan":
                envio.extend([4, 1])
            elif medicamento == "Amitriptilina":
                envio.extend([4, 2])
            elif medicamento == "Tianina":
                envio.extend([4, 3])
            elif medicamento == "Decatylen":
                envio.extend([4, 4])
            print(medicamento)
        envio_str = str(envio).replace(",", "").replace("[", "'").replace("]", "'")
        print(envio_str)
        
        TCP_IP = '192.168.173.254'
        TCP_PORT = 10000

        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((TCP_IP, TCP_PORT))
        s.send(envio_str.encode())
        time.sleep(3)
        s.close()
            
        messagebox.showinfo("Éxito", "Receta guardada y enviada exitosamente.")
        self.medicamentos_entry.config(state="normal")
        self.medicamentos_entry.delete(0, "end")
        

#if __name__ == "__main__":
#    root = tk.Tk()
#    app = InterfazRecetas(root)
#    root.mainloop()

root = tk.Tk()
app = InterfazRecetas(root)
root.mainloop()











