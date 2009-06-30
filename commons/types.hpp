#ifndef _TYPES_HPP_
#define _TYPES_HPP_

/*** Used for conditional compilation based on K3L version ***/

#define K3L_AT_LEAST(major,minor,build) \
    (((k3lApiMajorVersion == major) && ((k3lApiMinorVersion == minor) && (k3lApiBuildVersion >= build)) || \
     ((k3lApiMajorVersion == major) && (k3lApiMinorVersion > minor))) || \
      (k3lApiMajorVersion  > major))

#define K3L_EXACT(major,minor,build) \
    ((k3lApiMajorVersion == major) && (k3lApiMinorVersion == minor) && (k3lApiBuildVersion >= build))

#endif /* _TYPES_HPP_ */
