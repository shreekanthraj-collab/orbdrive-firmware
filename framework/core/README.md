# Framework Core

## Purpose

The Framework Core provides the common infrastructure shared by every Orb Drive firmware product.

This component defines:

- Fundamental firmware types
- Status and error codes
- Lifecycle stages
- Version information
- Assertions
- Common initialization entry points

Core is the lowest layer of the framework and must not depend on any higher-level component.

## Directory Structure

```
include/
```
Public API.

```
private/
```
Internal implementation.

```
src/
```
Source code.

```
test/
```
Unit tests.

## Version

v0.1.0
