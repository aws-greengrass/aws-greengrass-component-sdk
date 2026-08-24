# Greengrass Component SDK Developer setup

This guide is intended for developers working on the Greengrass Component SDK
codebase itself.

## Using Nix

Using Nix will allow you to use a reproducible development environment matching
CI as well as run the CI checks locally.

If you don't already have Nix, see the `Install Nix` section in
`../.github/workflows/ci.yml` to get the same version of Nix as used in CI.

To run all the formatters used by this project, run `nix fmt` in the project
root directory. `nix fmt <filepath>` can format a single file.

Note that untracked git files will be formatted as well, so if using build
directories or other files not tracked by git or in gitignore, add them to your
`./.git/info/exclude`.

To reproduce the CI checks locally, run `nix flake check -L`. Ensure this passes
for each commit in your PRs.

If making a PR to main, you can check all of your branches commits with
`git rebase main -x "nix flake check -L"`.

## Running Coverity

After installing Coverity and adding its bin dir to your path, run the following
in the project root dir:

```sh
cmake -B build
coverity scan
```

The html output will be in `build/cov-out`.

## Releasing

The following is the process for cutting a release of this repo. Use previous
releases as examples.

1. Ensure you have credentials to upload the crate to crates.io.
2. Ensure CI is green on main branch.
3. Ensure all new features have been added to all the bindings.
4. Make a release PR updating just the release notes and versions.
   1. Add a new section to top of release notes with sections with lists of new
      features and bug fixes customers should be made aware of. Skip mentioning
      commits which don't have customer impact.
   2. Update the version in `rust/Cargo.toml` and run cargo to update the
      version in `rust/Cargo.lock` accordingly.
   3. Use the format "Release vX.Y.Z" for the PR description and you can leave
      PR body blank.
5. Merge the release PR when ready.
6. Tag the release with an annotated tag (`-a` flag to `git tag`).
   1. Run `git tag -a vX.Y.Z`
   2. For the tag message, use "vX.Y.Z release" for the title, and copy the new
      release notes section into the description. Remove the markdown markup
      from the copied release notes (lists are fine). Note that this should be
      79 col wrapped to match git conventions. See previous tags for examples.
   3. Push the tag after verifying it is an annotated tag.
7. Make a Github release from the tag. Use same title and body as the tag
   message. Note that Github does not handle line wrapping, so unwrap the lines.
8. Publish the crate to crates.io using `nix run .#publish-rust-crate`. You must
   use that command or else the uploaded crate will not work.
9. Make PR in aws-greengrass-lite to update the SDK version.
