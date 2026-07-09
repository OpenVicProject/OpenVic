from git_fetcher import fetch_godot_files
from misc.utility.color import Ansi


def print_error(error: str) -> None:
    print(f"{Ansi.RED}{Ansi.BOLD}ERROR:{Ansi.REGULAR} {error}{Ansi.RESET}")


if not fetch_godot_files():
    print_error("Downloading Godot XML class references failed. Type references will fail.")
