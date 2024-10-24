/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

template <typename A, typename B>
__attribute__((noinline)) std::vector<B>
    *arrayMap(std::vector<A> *in, std::function<B(A, double)> cb) {
  auto *res = new std::vector<B>();
  for (double i = 0, e = in->size(); i < e; ++i)
    res->push_back(cb(in->at(i), i));
  return res;
}

template <typename T>
__attribute__((noinline)) std::vector<T>
    *arrayFilter(std::vector<T> *in, std::function<bool(T)> cb) {
  auto *res = new std::vector<T>();
  for (double i = 0, e = in->size(); i < e; ++i) {
    T el = in->at(i);
    if (cb(el))
      res->push_back(el);
  }
  return res;
}

template <typename T>
__attribute__((noinline)) bool arrayIncludes(std::vector<T> *in, T t) {
  for (double i = 0, e = in->size(); i < e; ++i)
    if (in->at(i) == t)
      return true;
  return false;
}

template <typename T>
__attribute__((noinline)) void arrayForEach(
    std::vector<T> *in,
    std::function<void(T)> cb) {
  for (double i = 0, e = in->size(); i < e; ++i)
    cb(in->at(i));
}

template <typename T>
__attribute__((noinline)) std::vector<T>
    *arrayConcat(std::vector<T> *arr1, std::vector<T> *arr2) {
  auto *result = new std::vector<T>();
  for (double i = 0, e = arr1->size(); i < e; ++i)
    result->push_back(arr1->at(i));
  for (double i = 0, e = arr2->size(); i < e; ++i)
    result->push_back(arr2->at(i));
  return result;
}

class Component {
  virtual void rtti() {}
};

class NumberComponent : public Component {
 public:
  double x;

  explicit NumberComponent(double x) : x(x) {}
};

class StringComponent : public Component {
 public:
  const char *x;

  explicit StringComponent(const char *x) : x(x) {}
};

class RenderNode;

class Context;

class Widget {
 public:
  std::optional<const char *> key;

  virtual RenderNode *reduce(Context *ctx) = 0;

  virtual const char *getName() {
    return "Widget";
  }
};

class Context {
 public:
  const char *key;
  double childCounter = 0;

  explicit Context(const char *key) : key(key) {}

  static Context *createForChild(Context *parentCtx, Widget *child) {
    auto widgetKey = child->key;
    const char *childKey;
    if (widgetKey)
      childKey = *widgetKey;
    else {
      auto *name = typeid(*child).name();
      // +1 for null terminator
      char *buf = (char *)malloc(strlen(name) + 11);
      sprintf(buf, "%s_%d", name, parentCtx->childCounter++);
      childKey = buf;
    }
    // +1 for underscore, +1 for null terminator
    char *newKey =
        (char *)malloc(strlen(parentCtx->key) + strlen(childKey) + 2);
    sprintf(newKey, "%s_%s", parentCtx->key, childKey);
    return new Context(newKey);
  }
};

using VirtualEntity = std::pair<double, std::vector<Component *> *>;

class RenderNode {
 public:
  const char *key;
  double id;
  std::vector<Component *> *components;
  std::vector<RenderNode *> *children;
  inline static double idCounter = 0;

  explicit RenderNode(
      const char *key,
      double id,
      std::vector<Component *> *components,
      std::optional<std::vector<RenderNode *> *> children)
      : key(key),
        id(id),
        components(components),
        children(children ? *children : new std::vector<RenderNode *>()) {}

  std::vector<VirtualEntity *> *reduce() {
    auto *flat = new std::vector<VirtualEntity *>();
    for (auto *child : *children)
      for (auto *v : *child->reduce())
        flat->push_back(v);
    return arrayConcat(
        new std::vector<VirtualEntity *>{new VirtualEntity{id, components}},
        flat);
  }

  static RenderNode *create(
      Context *ctx,
      std::vector<Component *> *components,
      std::optional<std::vector<RenderNode *> *> children) {
    return new RenderNode(ctx->key, idCounter++, components, children);
  }

  static RenderNode *createForChild(Context *ctx, Widget *child) {
    auto *childCtx = Context::createForChild(ctx, child);
    return child->reduce(childCtx);
  }
};

class ComposedWidget : public Widget {
 public:
  virtual Widget *render() = 0;

  RenderNode *reduce(Context *ctx) override {
    auto *child = render();
    return child->reduce(ctx);
  }
};

class Button : public Widget {
 public:
  double num;

  explicit Button(double num) : num(num) {}

  RenderNode *reduce(Context *ctx) override {
    auto *component = new NumberComponent(num);
    auto *components = new std::vector<Component *>();
    components->push_back(component);
    return RenderNode::create(ctx, components, std::nullopt);
  }
};

class Floater : public Widget {
 public:
  double num;

  explicit Floater(double num) : num(num) {}

  RenderNode *reduce(Context *ctx) override {
    auto *component = new NumberComponent(num);
    auto *components = new std::vector<Component *>();
    components->push_back(component);
    return RenderNode::create(ctx, components, std::nullopt);
  }
};

class Gltf : public Widget {
 public:
  const char *path;

  explicit Gltf(const char *path) : path(path) {}

  RenderNode *reduce(Context *ctx) override {
    auto *component = new StringComponent(path);
    auto *components = new std::vector<Component *>();
    components->push_back(component);
    return RenderNode::create(ctx, components, std::nullopt);
  }
};

class Container : public Widget {
 public:
  std::vector<Widget *> *children;

  explicit Container(std::vector<Widget *> *children) : children(children) {}

  RenderNode *reduce(Context *ctx) override {
    auto *component = new NumberComponent(13);
    auto *mappedChildren = arrayMap<Widget *, RenderNode *>(
        children, [ctx](Widget *child, double) {
          return RenderNode::createForChild(ctx, child);
        });
    auto *components = new std::vector<Component *>();
    components->push_back(component);
    return RenderNode::create(ctx, components, mappedChildren);
  }
};

struct RenderData {
  const char *modelPath;
  double buttonSize;
};

class ButtonAndModel : public ComposedWidget {
 public:
  RenderData *data;

  explicit ButtonAndModel(RenderData *data) : data(data) {}

  Widget *render() override {
    auto *children = new std::vector<Widget *>();
    children->push_back(new Button(data->buttonSize));
    children->push_back(new Gltf(data->modelPath));
    children->push_back(new Floater(sqrt(data->buttonSize)));

    return new Container(children);
  }
};

static std::vector<double> SIZES_SMALL({64, 8, 8, 18, 7, 24, 84, 4, 29, 58});
static std::vector<const char *> MODELS_SMALL(
    {"sazGTSGrfY",
     "uEQjieLDUq",
     "jQKzwhnzYa",
     "buIwVjnNDI",
     "goBJPxAkFf",
     "uKihCBaMwm",
     "VAyeIqqnSU",
     "bMNULcHsKb",
     "NBMEpcDimq",
     "wMCIoQQbNg"});

static std::vector<double> SIZES_LARGE({64, 8,  8,  18, 7,  99, 9,  61, 27,
                                        58, 30, 27, 95, 49, 37, 28, 87, 60,
                                        95, 1,  58, 14, 90, 9,  57});
static std::vector<const char *> MODELS_LARGE(
    {"sazGTSGrfY", "uEQjieLDUq", "jQKzwhnzYa", "buIwVjnNDI", "goBJPxAkFf",
     "oCtHzjLczM", "GJVcmhfddz", "nbpEAplbzQ", "yNXDBUcDys", "IZZQpqwiXa",
     "AAroNFkOBf", "flsXwiIaQG", "qazjSVkFcR", "PefkCqwfKJ", "yJDvizIEDY",
     "XauGblPeuo", "ZnvLBVjEom", "UeosvyfoBE", "BFeZAIAHQq", "iOYUWXWXhr",
     "WpOfaJwOlm", "sVHOxutIGB", "qOikwyZSWx", "KJGEPQxUKU", "cqrRvLCCYB"});

class TestApp : public ComposedWidget {
 public:
  bool renderLarge;

  explicit TestApp(bool renderLarge) : renderLarge(renderLarge) {}

  std::vector<Widget *> *getWidgets(
      std::vector<double> *sizes,
      std::vector<const char *> *models) {
    if (sizes->size() != models->size())
      throw 11;

    return arrayMap<
        const char *,
        Widget *>(models, [sizes](const char *modelPath, double i) {
      double buttonSize = sizes->at(i);
      auto *widget = new ButtonAndModel(new RenderData{modelPath, buttonSize});
      char *key = (char *)malloc(strlen(modelPath) + 12);
      sprintf(key, "%s_%d", modelPath, buttonSize);
      widget->key = key;
      return widget;
    });
  }

  std::vector<Widget *> *getChildren() {
    if (renderLarge)
      return getWidgets(&SIZES_LARGE, &MODELS_LARGE);
    return getWidgets(&SIZES_SMALL, &MODELS_SMALL);
  }

  Widget *render() override {
    auto *children = getChildren();
    return new Container(children);
  }
};

using ComponentPair = std::pair<double, Component *>;
struct SceneDiff {
  std::vector<double> *createdEntities;
  std::vector<double> *deletedEntities;
  std::vector<ComponentPair *> *createdComponents;
  std::vector<ComponentPair *> *deletedComponents;
};

std::vector<RenderNode *> *reconcileChildren(
    std::vector<RenderNode *> *newChildren,
    std::vector<RenderNode *> *oldChildren);

RenderNode *reconcileRenderNode(RenderNode *newNode, RenderNode *oldNode) {
  return new RenderNode(
      newNode->key,
      oldNode->id,
      newNode->components,
      reconcileChildren(newNode->children, oldNode->children));
}

std::vector<RenderNode *> *reconcileChildren(
    std::vector<RenderNode *> *newChildren,
    std::vector<RenderNode *> *oldChildren) {
  auto *outChildren = new std::vector<RenderNode *>();
  std::unordered_map<std::string_view, RenderNode *> oldChildrenByKey;
  arrayForEach<RenderNode *>(
      oldChildren, [&oldChildrenByKey](RenderNode *child) {
        oldChildrenByKey.emplace(child->key, child);
      });

  arrayForEach<RenderNode *>(
      newChildren, [&oldChildrenByKey, outChildren](RenderNode *child) {
        const char *newKey = child->key;
        auto it = oldChildrenByKey.find(newKey);
        if (it != oldChildrenByKey.end())
          outChildren->push_back(reconcileRenderNode(child, it->second));
        else
          outChildren->push_back(child);
      });
  return outChildren;
}

struct MapVector {
  std::unordered_map<double, std::vector<Component *> *> map;
  std::vector<std::pair<const double, std::vector<Component *> *> *> vec;
};

__attribute__((noinline)) void mapVectorForEach(
    MapVector *mv,
    std::function<void(double, std::vector<Component *> *)> cb) {
  for (double i = 0, e = mv->vec.size(); i < e; ++i) {
    auto [k, v] = *mv->vec.at(i);
    cb(k, v);
  }
}

MapVector *mapEntitiesToComponents(std::vector<VirtualEntity *> *entities) {
  auto *map = new MapVector();
  arrayForEach<VirtualEntity *>(entities, [map](VirtualEntity *entity) {
    double key = entity->first;
    auto *value = entity->second;

    auto it = map->map.find(key);

    if (it == map->map.end()) {
      it = map->map.emplace(key, new std::vector<Component *>()).first;
      map->vec.push_back(&*it);
    }

    auto *components = it->second;

    for (auto *v : *value)
      components->push_back(v);
  });

  return map;
}

SceneDiff *diffTrees(
    std::vector<VirtualEntity *> *newEntities,
    std::vector<VirtualEntity *> *oldEntities) {
  auto *createdComponents = new std::vector<ComponentPair *>();
  auto *deletedComponents = new std::vector<ComponentPair *>();

  auto *oldEntityIds = arrayMap<VirtualEntity *, double>(
      oldEntities, [](VirtualEntity *entity, double) { return entity->first; });
  auto *newEntityIds = arrayMap<VirtualEntity *, double>(
      newEntities, [](VirtualEntity *entity, double) { return entity->first; });

  auto *createdEntities = arrayFilter<double>(
      newEntityIds,
      [oldEntityIds](double id) { return !arrayIncludes(oldEntityIds, id); });
  auto *deletedEntities = arrayFilter<double>(
      oldEntityIds,
      [newEntityIds](double id) { return !arrayIncludes(newEntityIds, id); });

  auto *oldComponents = mapEntitiesToComponents(oldEntities);
  auto *newComponents = mapEntitiesToComponents(newEntities);

  arrayForEach<double>(
      createdEntities, [newComponents, createdComponents](double entityId) {
        auto it = newComponents->map.find(entityId);
        if (it == newComponents->map.end())
          return;
        auto *components = arrayMap<Component *, ComponentPair *>(
            it->second, [entityId](Component *component, double) {
              return new ComponentPair{entityId, component};
            });
        for (double i = 0, e = components->size(); i < e; ++i)
          createdComponents->push_back(components->at(i));
      });

  mapVectorForEach(
      newComponents,
      [oldComponents, createdComponents, deletedComponents](
          double key, auto *value) {
        if (!oldComponents->map.count(key))
          return;

        auto it = oldComponents->map.find(key);
        auto *oldComponentsForKey = it != oldComponents->map.end()
            ? it->second
            : new std::vector<Component *>();
        auto *newComponentsForKey = value;

        auto *deleted = arrayFilter<Component *>(
            oldComponentsForKey, [newComponentsForKey](Component *component) {
              return !arrayIncludes(newComponentsForKey, component);
            });
        auto *created = arrayFilter<Component *>(
            newComponentsForKey, [oldComponentsForKey](Component *component) {
              return !arrayIncludes(oldComponentsForKey, component);
            });

        arrayForEach<Component *>(
            deleted, [key, deletedComponents](Component *component) {
              deletedComponents->push_back(new ComponentPair{key, component});
            });
        arrayForEach<Component *>(
            created, [key, createdComponents](Component *component) {
              createdComponents->push_back(new ComponentPair{key, component});
            });
      });
  return new SceneDiff{
      createdEntities, deletedEntities, createdComponents, deletedComponents};
}

std::optional<SceneDiff *> runTest(bool includeTreeSerialization) {
  auto *oldCtx = new Context("root");
  auto *oldWidgetTree = (new TestApp(false))->render();
  auto *oldRenderTree = oldWidgetTree->reduce(oldCtx);
  auto *oldEntityTree = oldRenderTree->reduce();

  auto *newCtx = new Context("root");
  auto *newWidgetTree = (new TestApp(true))->render();
  auto *newRenderTree = newWidgetTree->reduce(newCtx);

  auto *reconciledRenderTree =
      reconcileRenderNode(newRenderTree, oldRenderTree);

  auto *reconciledEntityTree = reconciledRenderTree->reduce();
  auto *diff = diffTrees(reconciledEntityTree, oldEntityTree);

  if (includeTreeSerialization)
    return diff;
  return std::nullopt;
}

static constexpr bool printDiff = false;

void printEntities(std::vector<double> *entities) {
  std::cout << "[";
  for (double d : *entities)
    std::cout << d << ", ";
  std::cout << "]";
}

void printComponents(std::vector<ComponentPair *> *components) {
  std::cout << "[";
  for (ComponentPair *cp : *components) {
    std::cout << "[" << cp->first << ",";
    if (auto *nc = dynamic_cast<NumberComponent *>(cp->second))
      std::cout << nc->x;
    else
      std::cout << dynamic_cast<StringComponent *>(cp->second)->x;
    std::cout << "], ";
  }
  std::cout << "]";
}

int main() {
  if (printDiff) {
    SceneDiff *res = *runTest(true);
    std::cout << "{\n"
              << "createdEntities: ";
    printEntities(res->createdEntities);
    std::cout << "\ndeletedEntities: ";
    printEntities(res->deletedEntities);
    std::cout << "\ncreatedComponents: ";
    printComponents(res->createdComponents);
    std::cout << "\ndeletedComponents: ";
    printComponents(res->deletedComponents);
    std::cout << "\n}";
  } else {
    double i;
    for (i = 0; i < 50; ++i)
      runTest(false);
    // The actual execution.
    auto t1 = std::chrono::steady_clock::now();
    for (i = 0; i < 5000; ++i)
      runTest(false);
    auto t2 = std::chrono::steady_clock::now();
    auto t = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
    printf("%llu ms %d iterations\n", t.count(), (int)i);
  }
}
