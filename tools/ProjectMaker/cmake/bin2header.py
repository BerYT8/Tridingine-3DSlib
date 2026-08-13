import sys
from pathlib import Path


def main():
    if len(sys.argv) != 3:
        print("Uso: bin2header.py <input> <output>")
        return 1

    input_file = Path(sys.argv[1])
    output_file = Path(sys.argv[2])

    data = input_file.read_bytes()

    with output_file.open("w", encoding="ascii") as f:
        f.write("#pragma once\n\n")
        f.write("#include <cstddef>\n\n")

        f.write("static const unsigned char template_pak[] = {\n")

        for i in range(0, len(data), 16):
            chunk = data[i:i + 16]

            f.write("    ")

            f.write(
                ", ".join(
                    f"0x{byte:02X}"
                    for byte in chunk
                )
            )

            f.write(",")

            f.write("\n")

        f.write("};\n\n")

        f.write(
            "static const std::size_t template_pak_size "
            "= sizeof(template_pak);\n"
        )

    print(
        f"Generado {output_file} "
        f"({len(data)} bytes)"
    )

    return 0


if __name__ == "__main__":
    sys.exit(main())