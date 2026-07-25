#!/usr/bin/env python3
import tkinter as tk
from tkinter import filedialog, messagebox
import threading
import requests
import sys

class UploadGUI:
    def __init__(self, root):
        self.root = root
        root.title("C64 PRG Upload")
        root.resizable(True, True)

        # IP
        tk.Label(root, text="ESP32 IP:").grid(row=0, column=0, sticky="w", padx=5, pady=2)
        self.ip_entry = tk.Entry(root, width=20)
        self.ip_entry.grid(row=0, column=1, sticky="ew", padx=5, pady=2)

        # Start address (optional)
        tk.Label(root, text="Start Addr (hex, optional):").grid(row=1, column=0, sticky="w", padx=5, pady=2)
        self.start_entry = tk.Entry(root, width=20)
        self.start_entry.grid(row=1, column=1, sticky="ew", padx=5, pady=2)

        # Program file
        tk.Label(root, text="PRG File:").grid(row=2, column=0, sticky="w", padx=5, pady=2)
        self.file_frame = tk.Frame(root)
        self.file_frame.grid(row=2, column=1, sticky="ew", padx=5, pady=2)
        self.file_path = tk.StringVar()
        self.file_entry = tk.Entry(self.file_frame, textvariable=self.file_path, state="readonly")
        self.file_entry.pack(side="left", fill="x", expand=True)
        tk.Button(self.file_frame, text="Browse...", command=self._browse_file).pack(side="right", padx=2)

        # Upload button
        self.upload_btn = tk.Button(root, text="Upload", command=self._start_upload, width=15)
        self.upload_btn.grid(row=3, column=0, columnspan=2, pady=6)

        # Log output
        self.log_text = tk.Text(root, height=15, state="normal", wrap="word")
        self.log_text.grid(row=4, column=0, columnspan=2, sticky="nsew", padx=5, pady=2)

        # Scrollbar for log
        scrollbar = tk.Scrollbar(root, command=self.log_text.yview)
        scrollbar.grid(row=4, column=2, sticky="ns")
        self.log_text.config(yscrollcommand=scrollbar.set)

        # Grid weights
        root.columnconfigure(1, weight=1)
        root.rowconfigure(4, weight=1)

        self._log("Ready. Enter ESP32 IP and select a PRG file.\n")

    def _log(self, msg):
        self.log_text.insert(tk.END, msg)
        self.log_text.see(tk.END)
        self.root.update_idletasks()

    def _browse_file(self):
        path = filedialog.askopenfilename(
            title="Select PRG file",
            filetypes=[("PRG files", "*.prg"), ("All files", "*.*")]
        )
        if path:
            self.file_path.set(path)

    def _start_upload(self):
        ip = self.ip_entry.get().strip()
        program_file = self.file_path.get()
        start_str = self.start_entry.get().strip()

        if not ip:
            messagebox.showerror("Error", "ESP32 IP is required")
            return
        if not program_file:
            messagebox.showerror("Error", "Please select a PRG file")
            return

        start_address = None
        if start_str:
            try:
                start_address = int(start_str, 16)
            except ValueError:
                messagebox.showerror("Error", "Invalid start address – must be hex (e.g. 0801)")
                return

        self.upload_btn.config(state="disabled")
        thread = threading.Thread(target=self._do_upload, args=(ip, start_address, program_file), daemon=True)
        thread.start()

    def _do_upload(self, ip, start_address, program_file):
        try:
            with open(program_file, 'rb') as f:
                data = f.read()

            if len(data) >= 2:
                file_address = int.from_bytes(data[0:2], 'little')
                if start_address is None:
                    start_address = file_address
                    self._log(f"Read load address from file: 0x{start_address:04x}\n")
                elif file_address != start_address:
                    self._log(f"Warning: File load address (0x{file_address:04x}) != specified (0x{start_address:04x})\n")
                    self._log("Using file's load address...\n")
                    start_address = file_address
                program_data = data[2:]
            else:
                if start_address is None:
                    self._log("Error: File too small to contain a load address\n")
                    self.root.after(0, lambda: messagebox.showerror("Error", "File too small to contain a load address"))
                    return
                program_data = data

            hex_data = program_data.hex()
            url = f"http://{ip}/upload_c64_program"
            params = {'start': f"{start_address:04x}", 'data': hex_data}

            self._log(f"Uploading {len(program_data)} bytes to 0x{start_address:04x} ...\n")
            self._log(f"URL: {url}\n")

            response = requests.post(url, data=params, timeout=30)
            self._log(f"Response: {response.status_code}\n")
            self._log(f"Message: {response.text}\n")

        except FileNotFoundError:
            self._log(f"Error: File not found – {program_file}\n")
            self.root.after(0, lambda: messagebox.showerror("Error", f"File not found:\n{program_file}"))
        except requests.exceptions.RequestException as e:
            self._log(f"Error: {e}\n")
            self.root.after(0, lambda: messagebox.showerror("Error", str(e)))
        except Exception as e:
            self._log(f"Error: {e}\n")
            self.root.after(0, lambda: messagebox.showerror("Error", str(e)))
        finally:
            self.root.after(0, lambda: self.upload_btn.config(state="normal"))


def main():
    root = tk.Tk()
    UploadGUI(root)
    root.mainloop()


if __name__ == "__main__":
    main()
