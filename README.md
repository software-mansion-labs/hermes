# Static Hermes for React Native early preview

This repository is meant to open you the opportunity to use an early preview of new features from the static Hermes in your React Native application. Here are the instructions on how to use it. You can find the original Hermes Readme here: [Hermes Readme](https://github.com/software-mansion-labs/hermes/blob/main/README.md)

## Use prebuilt

### 1. Apply patch to `node_modules/react-native/sdks/hermes-engine/hermes-utils.rb`
```diff
--- hermes-utils.rb
+++ hermes-utils.rb
@@ -204,7 +204,7 @@
 def release_tarball_url(version, build_type)
     # Sample url from Maven:
     # https://repo1.maven.org/maven2/com/facebook/react/react-native-artifacts/0.71.0/react-native-artifacts-0.71.0-hermes-ios-debug.tar.gz
-    return "https://repo1.maven.org/maven2/com/facebook/react/react-native-artifacts/#{version}/react-native-artifacts-#{version}-hermes-ios-#{build_type.to_s}.tar.gz"
+    return "https://github.com/software-mansion-labs/hermes/releases/download/preview-v1/react-native-artifacts-hermes-ios-#{build_type.to_s}.tar.gz"
 end
 
 def download_stable_hermes(react_native_path, version, configuration)
```

### 2. Rebuild your `Pods`
```bash
cd ios && rm -r Pods && pod install
```

### 3. Just build & start your app

## Build from source

### 1. Build iOS framework
```bash
git clone git@github.com:software-mansion-labs/hermes.git
cd hermes
git checkout static-hermes-test
export DEBUG=true && ./utils/build-ios-framework.sh # set DEBUG=<true|false> depend on your build type
```

### 2. Update binaries in your app
Replace the directory `destroot` located at `YourAwesomeApp/ios/Pods/hermes-engine/destroot` in your application with the directory `hermes/destroot` that you've just built.

### 3. Just build & start your app
