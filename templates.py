import enum
import pathlib
import subprocess
import sys


class Section(enum.Enum):
    include = enum.auto()
    h = enum.auto()
    c = enum.auto()


def main() -> int:
    own_path = pathlib.Path(__file__)
    for template_path in pathlib.Path().glob("*.ct"):
        need_run = False
        while True:
            final_h_path = template_path.with_suffix(".h")
            final_c_path = template_path.with_suffix(".c")
            is_result_exist = final_h_path.exists() and final_c_path.exists()
            if not is_result_exist:
                need_run = True
                break

            input_time = max(own_path.stat().st_mtime, template_path.stat().st_mtime)
            result_time = max(
                final_h_path.stat().st_mtime, final_c_path.stat().st_mtime
            )
            if result_time < input_time:
                need_run = True
            break

        if need_run:
            template = template_path.read_text()
            types: list[str] = []
            template_include: list[str] = []
            template_h: list[str] = []
            template_c: list[str] = []
            section = Section.include
            for i, line in enumerate(template.split("\n")):
                if i == 0:
                    assert line.startswith("/// ")
                    types = line[4:].split(",")
                elif line.strip() == "/// h":
                    section = Section.h
                elif line.strip() == "/// c":
                    section = Section.c
                else:
                    match section:
                        case Section.include:
                            template_include.append(line)
                        case Section.h:
                            template_h.append(line)
                        case Section.c:
                            template_c.append(line)

            def generate(original: str, src: list[str]) -> str:
                for i, t in enumerate(types):
                    result = "\n".join(src)
                    result = result.replace("$t$", t)
                    result = result.replace("$l$", t.lower())
                    t_n = t[0].upper() + t[1:]
                    result = result.replace("$n$", t_n)
                    if i == len(types) - 1:
                        original += result
                    else:
                        original += result + "\n\n"
                return original

            guard = str(final_h_path).upper().replace(".", "_")
            final_h = f"#ifndef {guard}\n#define {guard}\n\n"
            final_h += "\n".join(template_include) + "\n\n"
            final_h = generate(final_h, template_h)
            final_h += f"\n\n#endif // {guard}"
            final_c = f'#include "{final_h_path}"\n\n'

            _ = final_h_path.write_text(final_h)
            _ = final_c_path.write_text(generate(final_c, template_c))

            for path in [final_h_path, final_c_path]:
                _ = subprocess.run(
                    ["clang-format", "-i", str(path)], stdout=subprocess.DEVNULL
                )

            print(f"Regenerate {final_h_path} and {final_c_path} from {template_path}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
