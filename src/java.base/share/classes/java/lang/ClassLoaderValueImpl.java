package java.lang;

import java.lang.reflect.Modifier;
import java.lang.reflect.UndeclaredThrowableException;
import java.util.Objects;
import java.util.concurrent.ConcurrentHashMap;
import java.util.function.Function;
import java.util.function.Supplier;

import jdk.internal.loader.BootLoader;

final class ClassLoaderValueImpl<T> extends ClassLoaderValue<T> {

    private final Class<? extends T> valueClass;
    private final ClassLoader valueClassLoader;
    private final boolean valueClassFinal;

    ClassLoaderValueImpl(Class<? extends T> valueClass) {
        this.valueClass = Objects.requireNonNull(valueClass);
        this.valueClassLoader = checkEqualOrDescendant(getClass().getClassLoader(), valueClass.getClassLoader());
        this.valueClassFinal = Modifier.isFinal(valueClass.getModifiers());
    }

    @Override
    @SuppressWarnings("unchecked")
    public T get(ClassLoader classLoader) {
        ConcurrentHashMap<Object, Object> m = map(checkEqualOrDescendant(classLoader));

        while (true) {
            Object v = m.get(this);
            if (v instanceof Memoizer) {
                T value = ((Memoizer<T>) v).get();
                if (m.replace(this, v, value)) {
                    return value;
                }
                continue;
            }
            return valueClass.cast(v);
        }
    }

    @Override
    @SuppressWarnings("unchecked")
    public T putIfAbsent(ClassLoader classLoader, T value) {
        Objects.requireNonNull(value);
        ConcurrentHashMap<Object, Object> m = map(checkEqualOrDescendant(valueSubclassLoader(value)));

        while (true) {
            Object v = m.putIfAbsent(this, value);
            if (v != null) {
                if (v instanceof Memoizer) {
                    T mv = ((Memoizer<T>) v).get();
                    if (m.replace(this, v, mv)) {
                        return mv;
                    }
                    continue;
                }
                return valueClass.cast(v);
            }
            return null;
        }
    }

    @Override
    @SuppressWarnings("unchecked")
    public T computeIfAbsent(
            ClassLoader classLoader,
            Function<? super ClassLoader, ? extends T> mappingFunction) {
        Objects.requireNonNull(mappingFunction);
        ConcurrentHashMap<Object, Object> m = map(checkEqualOrDescendant(classLoader));
        Memoizer<T> memoizer = null;

        while (true) {
            Object v = m.get(this);
            if (v != null) {
                if (v instanceof Memoizer) {
                    T mv = ((Memoizer<T>) v).get();
                    if (m.replace(this, v, mv)) {
                        return mv;
                    }
                    continue;
                }
                return valueClass.cast(v);
            }
            if (memoizer == null) {
                memoizer = new Memoizer<>(this, classLoader, mappingFunction);
            }
            if (m.putIfAbsent(this, memoizer) == null) {
                return null;
            }
        }
    }

    @Override
    @SuppressWarnings("unchecked")
    public T remove(ClassLoader classLoader) {
        ConcurrentHashMap<Object, Object> m = map(checkEqualOrDescendant(classLoader));

        Object v = m.remove(this);
        if (v instanceof Memoizer) {
            return ((Memoizer<T>) v).get();
        }
        return valueClass.cast(v);
    }

    @Override
    @SuppressWarnings("unchecked")
    public boolean remove(ClassLoader classLoader, Object value) {
        Objects.requireNonNull(value);
        ConcurrentHashMap<Object, Object> m = map(checkEqualOrDescendant(classLoader));

        while (true) {
            Object v = m.get(this);
            if (v instanceof Memoizer) {
                T mv;
                try {
                    mv = ((Memoizer<T>) v).get();
                } catch (Throwable t) {
                    return false;
                }
                m.replace(this, v, mv);
                continue;
            }
            if (v == null || !value.equals(v)) {
                return false;
            }
            if (m.remove(this, value)) {
                return true;
            }
        }
    }

    private ClassLoader valueSubclassLoader(T value) {
        return valueClassFinal ? valueClassLoader : value.getClass().classLoader;
    }

    private ClassLoader checkEqualOrDescendant(final ClassLoader descendant) {
        return checkEqualOrDescendant(this.valueClassLoader, descendant);
    }

    private static ClassLoader checkEqualOrDescendant(final ClassLoader parent, final ClassLoader descendant) {
        ClassLoader child = descendant;
        if (child == parent) {
            return descendant;
        }
        while (child != null) {
            child = child.parent;
            if (child == parent) {
                return descendant;
            }
        }
        throw new IllegalArgumentException(
                "Class loader "
                        + (descendant != null ? descendant.nameAndId() : "bootstrap")
                        + " is not equal to or a descendent of "
                        + (parent != null ? parent.nameAndId() : "bootstrap"));
    }

    private static final class Memoizer<T> implements Supplier<T> {

        private ClassLoaderValueImpl<T> classLoaderValue;
        private ClassLoader classLoader;
        private Function<? super ClassLoader, ? extends T> mappingFunction;
        private volatile T value;
        private volatile Throwable throwable;
        private boolean supplying;

        private Memoizer(ClassLoaderValueImpl<T> classLoaderValue, ClassLoader classLoader,
                Function<? super ClassLoader, ? extends T> mappingFunction) {
            this.classLoaderValue = classLoaderValue;
            this.classLoader = classLoader;
            this.mappingFunction = mappingFunction;
        }

        @Override
        public T get() {
            T value = this.value;
            if (value != null) {
                return value;
            }
            Throwable throwable = this.throwable;
            if (throwable != null) {
                if (throwable instanceof Error) {
                    throw (Error) throwable;
                } else if (throwable instanceof RuntimeException) {
                    throw (RuntimeException) throwable;
                } else {
                    throw new UndeclaredThrowableException(throwable);
                }
            }
            synchronized (this) {
                return getLocked();
            }
        }

        private T getLocked() {
            T value = this.value;
            if (value != null) {
                return value;
            }
            Throwable throwable = this.throwable;
            if (throwable != null) {
                if (throwable instanceof Error) {
                    throw (Error) throwable;
                } else if (throwable instanceof RuntimeException) {
                    throw (RuntimeException) throwable;
                } else {
                    throw new UndeclaredThrowableException(throwable);
                }
            }
            if (supplying) {
                throw new IllegalStateException("Recursive invocation of " + mappingFunction.getClass() + "@"
                        + Integer.toHexString(System.identityHashCode(mappingFunction))
                        + " supplied to java.lang.ClassLoaderValue@ "
                        + Integer.toHexString(System.identityHashCode(classLoaderValue)) + "#computeIfAbsent");
            }
            supplying = true;
            try {
                value = Objects.requireNonNull(mappingFunction.apply(classLoader));
                classLoaderValue.checkEqualOrDescendant(classLoaderValue.valueSubclassLoader(value));
                this.value = value;
                classLoaderValue = null;
                classLoader = null;
                mappingFunction = null;
                return value;
            } catch (Throwable t) {
                this.throwable = t;
                classLoaderValue = null;
                classLoader = null;
                mappingFunction = null;
                if (t instanceof Error) {
                    throw (Error) t;
                } else if (t instanceof RuntimeException) {
                    throw (RuntimeException) t;
                } else {
                    throw new UndeclaredThrowableException(t);
                }
            } finally {
                supplying = false;
            }
        }
    }
}
