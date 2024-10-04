# Static Hermes for React Native – Experimental Builds

> This is a fork of [hermes](https://github.com/facebook/hermes/) repository. If you want to see the original README, head to the [upstream repository webside](https://github.com/facebook/hermes/)

This repository contains the necessary changes to build the new version of hermes (also known as static hermes) for React Native open-source apps.
We are collaborating with the hermes team to include these changes in the main repository and eventually publish the new Hermes as part of a future React Native release.

In the meantime, you can use this repository to try out the new hermes in your React Native project.
Currently, only iOS is supported.
You can build the new Hermes from source or use the pre-built package that we are distributing via the releases page on GitHub.
Please follow the instructions below based on the approach you choose, and don't hesitate to leave feedback or report issues here or on the [hermes](https://github.com/facebook/hermes/) repository if you encounter any crashes.

If you're interested in testing the new Hermes and evaluating its impact on your app's performance, we recommend testing with release builds configured to use bytecode compilation.
This means that instead of interpreting the bundle at runtime, the bundle is compiled into bytecode during the build process, which is then used to run your app.

The new hermes supports running JavaScript in various "modes": as an interpreter, typed native, untyped native, typed bytecode, and untyped bytecode.
The best performance is achieved when running typed code, meaning that instead of compiling JavaScript, hermes takes a typed source (TypeScript or Flow) as input.
However, with the current React Native setup, it is not yet possible to generate a typed bundle, so only untyped code can be provided to the compiler.
When following the instructions in this repository, your app will run in interpreter mode for Debug builds or in untyped bytecode mode for Release builds.

> [!WARNING]  
> This project and the included pre-built packages should be treated as experimental. While Meta has announced that they have been using the new hermes in production for some time, we do not recommend doing so until our changes are officially approved by the Hermes team.

## Use pre-built 

This instruction lets you use the new hermes without building it from sources.
It will use the bre-built hermes binaries that we are distributing via the releases page.

### 1. Apply the below patch to `node_modules/react-native/sdks/hermes-engine/hermes-utils.rb`
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

Note that this patches files under `node_module` directory, so if you re-install node modules after this step, or want to share it with others using your repo, you need to use some tool like [patch-package](https://github.com/ds300/patch-package).

### 2. Rebuild iOS `Pods`
```bash
cd ios && rm -r Pods && pod install
```

### 3. Build and start your app normally

After this step you should be all set.
Remember to use release builds if you want to try the untyped bytecode more of the new hermes.

## Building from source

If you don't want to use the provided binaries, you can use this repo to build the new hermes from source.

### 1. Build hermes iOS framework
```bash
git clone git@github.com:software-mansion-labs/hermes.git
cd hermes
git checkout static-hermes-test
export DEBUG=true && ./utils/build-ios-framework.sh # set DEBUG=<true|false> depend on your build type
```

### 2. Update binaries in your app
Replace the directory `destroot` located at `YourAwesomeApp/ios/Pods/hermes-engine/destroot` in your application with the directory `hermes/destroot` that you've just built.

Note that because you are replacing binary files located under Pods, those could get overridden by future `pod install` calls.
It is also likely that `Pods` folder is excluded from version control.

### 3. Build and start your app normally

After this step you should be all set.
Remember to use release builds if you want to try the untyped bytecode more of the new hermes.
