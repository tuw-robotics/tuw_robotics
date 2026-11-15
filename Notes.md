# Developer and AI Notes for tuw_robotics

## Releasing the packages

How to prepare and publish a new release of the `tuw_robotics` packages
(`tuw`, `tuw_json`, `tuw_rviz`, `tuw_gamepad` and the meta package `tuw_robotics`) with **bloom**.

The commands below use `$ROS_DISTRO`, which is set when you source the ROS 2 environment (e.g. by `env.sh`).
Check that it is the distribution you are releasing for (`echo $ROS_DISTRO`), e.g. `rolling`, `jazzy`, `lyrical`.
In the text and in the bloom answers, `<distro>` stands for the same value.
Each distribution has its own branch on GitHub, see [README.md](./README.md).

---

### 1. Code formatting

The packages follow the official ROS 2 code style with one exception: the **maximum line length is 200** instead of 100.

#### Style files

| File | Tool | Notes |
|---|---|---|
| [`.clang-format`](./.clang-format) | `clang-format`, `ament_clang_format`, editors | `ColumnLimit: 200` |
| [`ament_code_style.cfg`](./ament_code_style.cfg) | `ament_uncrustify` (Uncrustify 0.78) | `code_width = 200` |

Both files are copies of the official ROS 2 configuration from
[ament/ament_lint](https://github.com/ament/ament_lint), with only the line length changed.

> **Note:** The author overrode the maximum line length in the `CMakeLists.txt` of every package with tests
> (`tuw`, `tuw_json`, `tuw_rviz`, `tuw_gamepad`). Without this override, `colcon test` would use the ROS 2 default of
> 100 characters and fail on longer lines. Keep the override when you edit these files, and add it to any new package.

In every package's `CMakeLists.txt`, the default uncrustify check (100 characters) is replaced with one that allows 200 characters:

```cmake
list(APPEND AMENT_LINT_AUTO_EXCLUDE ament_cmake_uncrustify)
ament_lint_auto_find_test_dependencies()
find_package(ament_cmake_uncrustify REQUIRED)
ament_uncrustify(MAX_LINE_LENGTH 200)
```

`cpplint` is currently disabled (`set(ament_cmake_cpplint_FOUND TRUE)`).
If you enable it, also add `ament_cpplint(MAX_LINE_LENGTH 200)`.

#### Install the tools

```bash
sudo apt install clang-format ros-$ROS_DISTRO-ament-clang-format ros-$ROS_DISTRO-ament-uncrustify
```

#### Check the formatting (no changes)

Run in the repository root (`ws/src/tuw_robotics`):

```bash
ament_clang_format --config .clang-format .
ament_uncrustify -c ament_code_style.cfg .
```

#### Fix the formatting

Run in this order. Uncrustify runs last because it is the tool the tests use:

```bash
ament_clang_format --config .clang-format --reformat .
ament_uncrustify -c ament_code_style.cfg --reformat .
```

#### Run all linters and tests

Run in the workspace (`ws`):

```bash
colcon build --packages-select tuw tuw_json tuw_rviz tuw_gamepad tuw_robotics
colcon test  --packages-select tuw tuw_json tuw_rviz tuw_gamepad tuw_robotics
colcon test-result --verbose
```

All tests must pass before you release.

---

### 2. Release with bloom

#### Prerequisites (once per machine)

```bash
sudo apt install python3-bloom python3-catkin-pkg python3-rosdep
```

- **GitHub token:** On its first run, bloom asks for a GitHub personal access token with the scopes
  `public_repo` and `workflow`. It stores the token in `~/.config/bloom`.
- **Release repository:** bloom pushes to a release repository in the
  [ros2-gbp](https://github.com/ros2-gbp) organisation, `https://github.com/ros2-gbp/tuw_robotics-release.git`.
  If it does not exist yet, request it by opening an issue at
  [ros2-gbp/ros2-gbp-github-org](https://github.com/ros2-gbp/ros2-gbp-github-org/issues/new/choose).

#### Step 1: Prepare the release branch

```bash
cd ws/src/tuw_robotics
git checkout $ROS_DISTRO
git pull
git merge $ROS_DISTRO-devel          # if there is a devel branch for this distro
git status                        # working tree must be clean
```

#### Step 2: Format and test

Follow [section 1](#1-code-formatting): fix the formatting, then run all tests. Commit any formatting changes.

#### Step 3: Check package.xml

- All packages in this repository must have the **same version**. `catkin_prepare_release` refuses to run otherwise.
  At the time of writing, `tuw_gamepad` is at `0.0.0` and all other packages are at `0.0.2`.
- Every dependency must be a valid rosdep key. Check this with:

  ```bash
  rosdep update
  rosdep check --from-paths . --ignore-src --rosdistro $ROS_DISTRO
  ```

#### Step 4: Update the changelogs

```bash
catkin_generate_changelog          # first time for a package without CHANGELOG.rst: catkin_generate_changelog --all
```

This adds a "Forthcoming" section to each `CHANGELOG.rst`, built from the commit messages since the last tag.
Edit the entries so they make sense to users, then commit:

```bash
git commit -am "update changelogs"
```

#### Step 5: Bump the version and tag

```bash
catkin_prepare_release --bump patch     # or: --bump minor / --bump major
```

This command:

- bumps `<version>` in every `package.xml`
- replaces "Forthcoming" in the changelogs with the new version and date
- commits, creates the tag (e.g. `0.0.3`) and pushes both to GitHub

#### Step 6: Run bloom

```bash
bloom-release --rosdistro $ROS_DISTRO --track $ROS_DISTRO tuw_robotics
```

**First release for a distribution:** add `--new-track`. bloom then asks for:

| Question | Answer |
|---|---|
| Release repository | `https://github.com/ros2-gbp/tuw_robotics-release.git` |
| Repository name | `tuw_robotics` |
| Upstream repository URI | `https://github.com/tuw-robotics/tuw_robotics.git` |
| Upstream VCS type | `git` |
| Version | `:{auto}` (taken from `package.xml`) |
| Release tag | `:{version}` |
| Upstream devel branch | `<distro>` |
| ROS distro | `<distro>` |

Accept the defaults for everything else.

To change the track settings later:

```bash
bloom-release --rosdistro $ROS_DISTRO --track $ROS_DISTRO --edit tuw_robotics
```

At the end, bloom opens a pull request against [ros/rosdistro](https://github.com/ros/rosdistro).
Check it and follow up on any review comments.

#### Step 7: After the pull request is merged

1. The ROS buildfarm builds the binary packages. Follow the jobs on [build.ros2.org](https://build.ros2.org/).
2. The packages first appear in the **testing** repository (`ros2-testing`).
3. With the next sync of the distribution they move to the main repository, where users can install them:

   ```bash
   sudo apt install ros-$ROS_DISTRO-tuw-robotics
   ```

---

### Checklist

- [ ] On the `<distro>` branch, merged and pulled, working tree clean
- [ ] Formatting fixed (`ament_clang_format`, then `ament_uncrustify`)
- [ ] `colcon test` passes for all packages
- [ ] All packages have the same version, `rosdep check` passes
- [ ] Changelogs generated, edited and committed
- [ ] `catkin_prepare_release` ran: version bumped, tagged and pushed
- [ ] `bloom-release` ran, rosdistro pull request opened
- [ ] Pull request merged, buildfarm jobs green
