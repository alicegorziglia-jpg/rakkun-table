#!/usr/bin/env bash
set -euo pipefail

# Config
BRANCH="fix/ci-gradle-wrapper"
REMOTE="origin"

echo "Fetching origin..."
git fetch "$REMOTE"

# create branch from origin/main or reset if exists
if git show-ref --verify --quiet "refs/heads/$BRANCH"; then
  git checkout "$BRANCH"
  git reset --hard "$REMOTE/main"
else
  git checkout -b "$BRANCH" "$REMOTE/main"
fi

mkdir -p .github/workflows

cat > .github/workflows/build-apk.yml <<'YAML'
name: Build APK

on:
  push:
    branches: [ main ]
  pull_request:
    branches: [ main ]
  workflow_dispatch:

jobs:
  build:
    runs-on: ubuntu-latest

    steps:
      - name: Checkout código
        uses: actions/checkout@v4

      - name: Setup JDK 17
        uses: actions/setup-java@v4
        with:
          java-version: '17'
          distribution: 'temurin'

      - name: Setup Android SDK
        uses: android-actions/setup-android@v3

      - name: Limpiar caché potencialmente corrupto de Gradle
        run: |
          rm -rf ~/.gradle/caches/jars-*
          rm -rf ~/.gradle/caches/modules-*/files-*

      - name: Setup Gradle (instala Gradle 8.6 en el runner)
        uses: gradle/actions/setup-gradle@v3
        with:
          gradle-version: '8.6'
          cache-disabled: true

      - name: Ensure Gradle wrapper exists (generate if missing) and debug
        run: |
          echo "PWD: $(pwd)"
          echo "Listing repo root:"
          ls -la

          if [ ! -f android/gradlew ]; then
            echo "Gradle wrapper missing — generating with gradle in android/ ..."
            (cd android && gradle wrapper --gradle-version 8.6) || { echo "gradle wrapper failed"; ls -la android || true; exit 1; }
          else
            echo "Gradle wrapper already present"
          fi

          echo "Listing android/ after generation:"
          ls -la android || true
          echo "Listing android/gradle/wrapper/ :"
          ls -la android/gradle/wrapper || true

          chmod +x android/gradlew || true
          echo "android/gradlew permissions:"
          ls -l android/gradlew || true

      - name: Debug android dir
        run: ls -la android || true

      - name: Check Java and Gradle (wrapper) versions
        run: |
          java -version
          (cd android && ./gradlew --version)

      - name: Build debug APK (usa el wrapper del repo)
        run: ./gradlew assembleDebug --no-daemon --no-build-cache --refresh-dependencies --stacktrace --info
        working-directory: android

      - name: Upload APK
        uses: actions/upload-artifact@v4
        with:
          name: GraphicTablet-debug
          path: android/app/build/outputs/apk/debug/app-debug.apk
          retention-days: 30

      - name: Create Release (solo en tags v*)
        if: startsWith(github.ref, 'refs/tags/v')
        uses: softprops/action-gh-release@v1
        with:
          files: android/app/build/outputs/apk/debug/app-debug.apk
          name: GraphicTablet ${{ github.ref_name }}
        env:
          GITHUB_TOKEN: ${{ secrets.GITHUB_TOKEN }}
YAML

# Try to generate Gradle wrapper locally (only if gradle is available)
if command -v gradle >/dev/null 2>&1; then
  echo "Gradle detected locally: generating wrapper in android/ (gradle --version)"
  gradle --version | head -n 1 || true
  (cd android && gradle wrapper --gradle-version 8.6)
else
  echo "Gradle not found locally. Skipping local wrapper generation; CI will generate it on the runner."
fi

# Add files (safe: add what exists)
git add .github/workflows/build-apk.yml || true
git add android/gradlew android/gradlew.bat android/gradle/wrapper/gradle-wrapper.jar android/gradle/wrapper/gradle-wrapper.properties 2>/dev/null || true

# Mark gradlew executable if present
if [ -f android/gradlew ]; then
  git update-index --chmod=+x android/gradlew || true
fi

# Commit & push if there are changes
if [ -n "$(git status --porcelain)" ]; then
  git commit -m "CI: generate Gradle wrapper if missing, use Gradle 8.6 and run build with ./gradlew"
  git push --set-upstream "$REMOTE" "$BRANCH"
  echo "Pushed branch $BRANCH. Create a PR from it to main (or merge directly)."
else
  echo "No changes to commit. Branch is up-to-date."
fi

echo "Done."