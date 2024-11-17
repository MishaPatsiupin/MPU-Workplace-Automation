import os
import glob

def number_lines(file_path):
    with open(file_path, 'r') as file:
        lines = file.readlines()

    num_lines = len(lines)
    if num_lines < 100:
        line_format = "{:02d}. {}"
    else:
        line_format = "{:03d}. {}"

    numbered_lines = [line_format.format(i + 1, line) for i, line in enumerate(lines)]

    base_name, ext = os.path.splitext(file_path)
    new_file_path = f"{base_name}_{ext[1:]}.txt"
    with open(new_file_path, 'w') as new_file:
        new_file.writelines(numbered_lines)

def process_files():
    for file_path in glob.glob("*.h") + glob.glob("*.cpp"):
        number_lines(file_path)

if __name__ == "__main__":
    process_files()