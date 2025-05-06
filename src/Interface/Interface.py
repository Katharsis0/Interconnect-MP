import tkinter as tk
from tkinter import ttk
import matplotlib.pyplot as plt
from client import create_socket, connect_to_server, get_button_action, close_socket

class InterfazGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Interfaz Multiprocesador")
        self.root.geometry("1000x650")
        self.root.configure(bg="#F4EDD3")

        # Cliente socket
        self.sock = create_socket()
        connect_to_server(self.sock)

        # Título
        self.etiqueta = tk.Label(
            root,
            text="Visualización del sistema Multi-Procesador (MP)",
            font=("Roboto", 20, "italic", "bold"),
            bg="#F4EDD3", fg="#4C585B"
        )
        self.etiqueta.pack(pady=20)

        # Main container
        main_frame = tk.Frame(self.root, bg="#F4EDD3")
        main_frame.pack(fill="both", expand=True, padx=20, pady=10)

        # Izquierda
        self.left_frame = tk.Frame(main_frame, bg="#F4EDD3")
        self.left_frame.pack(side="left", fill="y", padx=10)

        self._crear_metricas()
        self._crear_botones_laterales()

        # Derecha
        self.right_frame = tk.Frame(main_frame, bg="#F4EDD3")
        self.right_frame.pack(side="right", fill="both", expand=True, padx=10)

        self._crear_pe_table()

    def _crear_metricas(self):
        label = tk.Label(self.left_frame, text="Métricas", font=("Segoe UI", 14, "bold"), bg="#F4EDD3", fg="#4C585B")
        label.pack(anchor="w")

        self.metrics_table = ttk.Treeview(self.left_frame, columns=("nombre", "valor"), show="headings", height=6)
        self.metrics_table.heading("nombre", text="Nombre")
        self.metrics_table.heading("valor", text="Valor")
        self.metrics_table.column("nombre", anchor="center", width=160)
        self.metrics_table.column("valor", anchor="center", width=100)
        self.metrics_table.pack(pady=10)

    def _crear_botones_laterales(self):
        self.etiqueta = tk.Label(self.root, text="Elige como ver\nlos pasos de los PE", font=("Roboto", 15),
                                 bg="#F4EDD3", fg="#4C585B")
        self.etiqueta.place(relx=0.15, rely=0.60, anchor="center")

        tk.Button(self.root, text="▶ step", font=("Roboto", 10, "bold"),
                  command=lambda: self.actualizar("step"),
                  width=15, height=2, bg="#4CAF50", fg="white",
                  borderwidth=1, relief="solid").place(relx=0.15, rely=0.75, anchor="center")

        tk.Button(self.root, text="automático [clk]", font=("Roboto", 10, "bold"),
                  command=lambda: self.actualizar("clk"),
                  width=15, height=2, bg="#7E99A3", fg="white",
                  borderwidth=1, relief="solid").place(relx=0.15, rely=0.85, anchor="center")

        tk.Button(self.root, text="Mostrar Gráfico", font=("Roboto", 10, "bold"),
                  command=lambda: self.showGraph(), width=25, height=2,
                  bg="#A5BFCC", fg="#000000",
                  borderwidth=1, relief="solid").place(relx=0.50, rely=0.9, anchor="center")

        tk.Button(self.root, text="reiniciar", font=("Roboto", 10, "bold"),
                  command=lambda: self.actualizar("reiniciar"),
                  width=15, height=2, bg="#A5BFCC", borderwidth=1, relief="solid").place(relx=0.85, rely=0.9, anchor="center")

    def _crear_pe_table(self):
        label = tk.Label(self.right_frame, text="Estado de PEs", font=("Segoe UI", 14, "bold"), bg="#F4EDD3", fg="#4C585B")
        label.pack(anchor="w")

        self.pe_table = ttk.Treeview(self.right_frame, columns=("ID", "Instrucción"), show="headings", height=10)
        self.pe_table.heading("ID", text="ID")
        self.pe_table.heading("Instrucción", text="Instrucción actual")
        self.pe_table.column("ID", anchor="center", width=80)
        self.pe_table.column("Instrucción", anchor="center", width=300)
        self.pe_table.pack(fill="x", pady=10)

    def actualizar(self, tipo):
        try:
            datos = get_button_action(self.sock, tipo)
            self._actualizar_metricas(datos["metricas"])
            self._actualizar_pe(datos["pe"])
        except Exception as e:
            print(f"Error al actualizar: {e}")

    def _actualizar_metricas(self, nuevas_metricas):
        for item in self.metrics_table.get_children():
            self.metrics_table.delete(item)
        for m in nuevas_metricas:
            self.metrics_table.insert('', tk.END, values=m)

    def _actualizar_pe(self, nuevos_pe):
        for item in self.pe_table.get_children():
            self.pe_table.delete(item)
        for pe in nuevos_pe:
            self.pe_table.insert('', tk.END, values=pe)

    def showGraph(self):
        nombres, valores = [], []

        for child in self.metrics_table.get_children():
            item = self.metrics_table.item(child)["values"]
            nombres.append(item[0])
            valores.append(item[1])

        plt.figure(figsize=(8, 5))
        plt.bar(nombres, valores, color="#4CAF50")
        plt.title("Métricas del sistema")
        plt.xlabel("Métrica")
        plt.ylabel("Valor")
        plt.xticks(rotation=20)
        plt.tight_layout()
        plt.show()

    def __del__(self):
        close_socket(self.sock)


if __name__ == "__main__":
    root = tk.Tk()
    app = InterfazGUI(root)
    root.mainloop()
