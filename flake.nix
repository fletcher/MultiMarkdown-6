{
  description = "MultiMarkdown-6 — lightweight markup processor";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        packages = {
          multimarkdown = pkgs.stdenv.mkDerivation {
            pname = "multimarkdown";
            version = "6.7.0";

            src = ./.;

            nativeBuildInputs = [ pkgs.cmake ];

            cmakeFlags = [
              "-DCMAKE_BUILD_TYPE=Release"
            ];

            meta = {
              description = "Lightweight markup processor to produce HTML, LaTeX, and more";
              homepage = "https://github.com/zcash/MultiMarkdown-6";
              license = pkgs.lib.licenses.mit;
              mainProgram = "multimarkdown";
            };
          };

          default = self.packages.${system}.multimarkdown;
        };
      }
    );
}
