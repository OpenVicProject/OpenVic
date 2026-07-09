#!/usr/bin/env python3

# This module handles downloading Godot XML files from GitHub without needing to clone the whole repository.
from __future__ import annotations

import os
import subprocess
from pathlib import Path
from typing import Any, List

REPO_OWNER_DEFAULT: str = "godotengine"
REPO_NAME_DEFAULT: str = "godot"
REF_DEFAULT: str = "4.7-stable"
GIT_REPO_TEMPLATE_DEFAULT: str = "https://github.com/{repo_owner}/{repo_name}"
CACHE_PATH_DEFAULT: str = "extension/doc_tools/cache/engine"


class GitFetcher:
    """Downloads Git files."""

    def __init__(
        self,
        repo_owner: str = REPO_OWNER_DEFAULT,
        repo_name: str = REPO_NAME_DEFAULT,
        ref: str = REF_DEFAULT,
        git_repo_template: str = GIT_REPO_TEMPLATE_DEFAULT,
    ):
        self.repo_owner = repo_owner
        self.repo_name = repo_name
        self.ref = ref
        self.git_repo = git_repo_template.format(**self.__dict__)

    def _call_git(self, *args: str, additional_cwd: str | None = None) -> int:
        if additional_cwd is None:
            additional_cwd = self.repo_name
        return subprocess.call(
            " ".join(["git", *args]),
            cwd=os.path.join(self.cache_path, additional_cwd),
            shell=True,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.STDOUT,
        )

    def _check_git_output(self, *args: str, additional_cwd: str | None = None) -> Any:
        if additional_cwd is None:
            additional_cwd = self.repo_name
        return subprocess.check_output(
            " ".join(["git", *args]), cwd=os.path.join(self.cache_path, additional_cwd), shell=True
        )

    def setup_sparse_checkout(
        self,
        dirs: List[str] = [
            "doc/classes/*.xml",
            "modules/*/doc_classes/*.xml",
            "platform/*/doc_classes/*.xml",
        ],
    ) -> bool:
        print(f"Downloading {self.repo_name.capitalize()} {self.ref} XML class references via Git...")

        cache_dir_path = Path(self.cache_path)
        cache_dir_path.mkdir(parents=True, exist_ok=True)

        ref_file_path = cache_dir_path / "ref"

        try:
            if not Path(cache_dir_path / self.repo_name).exists():
                raise subprocess.CalledProcessError(-1, "")

            if ref_file_path.exists():
                with ref_file_path.open() as ref_file:
                    if ref_file.read() == self.ref:
                        return True

            if self._call_git("fetch --depth=1", "origin", self.ref) != 0:
                print(f"Could not fetch {self.ref} of {self.git_repo}")
                return False

        except subprocess.CalledProcessError:
            if (
                self._call_git("clone", "--filter=blob:none", "--sparse", "--depth=1", self.git_repo, additional_cwd="")
                != 0
            ):
                print(f"Could not clone {self.git_repo}")
                return False

            if self._call_git("fetch --depth=1", "origin", self.ref) != 0:
                print(f"Could not fetch {self.ref} of {self.git_repo}")
                return False

            if self._call_git("sparse-checkout", "init") != 0:
                print("git sparse-checkout init failed")
                return False

            if self._call_git("sparse-checkout", "set", "--no-cone", *dirs) != 0:
                print(f"Could not set sparse-checkout directories to {dirs}")
                return False

        if self._call_git("checkout", "FETCH_HEAD") != 0:
            print(f"Could not checkout {self.git_repo}")
            return False

        with ref_file_path.open(mode="w") as ref_file:
            ref_file.write(self.ref)

        return True

    def fetch_all_documentation(self, cache_path: str = CACHE_PATH_DEFAULT) -> bool:
        """Download all documentation XML files."""
        self.cache_path = cache_path
        if self.setup_sparse_checkout():
            print("XML class references updated.")
            return True
        return False


def fetch_godot_files(ref: str = REF_DEFAULT, cache_path: str = CACHE_PATH_DEFAULT) -> bool:
    """Convenience function to fetch Godot files."""
    fetcher = GitFetcher(ref=ref)
    return fetcher.fetch_all_documentation(cache_path)


if __name__ == "__main__":
    fetch_godot_files()
