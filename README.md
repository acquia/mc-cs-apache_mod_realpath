# apache_mod_realpath
Apache2 module for resolving symlinks - allows you to easily use symlinks for application upgrades

# Building

```
apxs -c -i mod_realpath.c
```

# Usage

This modules exposes two functions. 

The first one (`realpath`) allows you to resolve all symlinks in a given path. That allows you to make application upgrades atomic as the application (php for example) will only see the resolved path (/var/www/app-1.0.1/index.php). Never the path with the symlink (/var/www/app-current/index.php).

``` 
RewriteRule ^/index.php$ "${realpath:/var/www/app-current/index.php}" [L]
```

The second function  (`owneruid`) will give you the UID of an owner of a given path. That can be very usefull in combination with mpm-itk. You can dynamically change the uid of the apache process to match the owner of target application:

```
AssignUserIDExpr #%{owneruid:/var/www/application}
AssignGroupIDExpr #%{owneruid:/var/www/application}

```
