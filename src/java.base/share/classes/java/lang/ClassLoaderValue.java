package java.lang;

import java.util.Objects;
import java.util.concurrent.ConcurrentHashMap;
import java.util.function.Function;

import jdk.internal.loader.BootLoader;

/**
 * Lazily associate a computed value with (potentially) every
 * {@link ClassLoader}.
 * 
 * @param <T> the type of the derived value
 */
public abstract sealed class ClassLoaderValue<T>
        permits ClassLoaderValueImpl, jdk.internal.loader.AbstractClassLoaderValue {

    /**
     * Returns a new instance of {@link ClassLoaderValue}.
     * 
     * @param valueClass class of the value
     * @return ClassLoaderValue
     * @param <T> the type of the derived value
     */
    public static <T> ClassLoaderValue<T> of(Class<T> valueClass) {
        return new ClassLoaderValueImpl<>(Objects.requireNonNull(valueClass));
    }

    /**
     * Gets the value associated with this ClassLoaderValue.
     * 
     * @param classLoader ClassLoader the value is assocaited with
     * @return value assocaited with the ClassLoader
     */
    public abstract T get(ClassLoader classLoader);

    /**
     * Gets the value associated with this ClassLoaderValue.
     * 
     * @param classLoader ClassLoader the value is assocaited with
     * @param value       value to associate with the ClassLoader
     * @return value assocaited with the ClassLoader
     */
    public abstract T putIfAbsent(ClassLoader classLoader, T value);

    /**
     * Gets the value associated with this ClassLoaderValue.
     * 
     * @param classLoader     ClassLoader the value is assocaited with
     * @param mappingFunction mapping function used to compute the value if it is
     *                        absent
     * @return value assocaited with the ClassLoader
     */
    public abstract T computeIfAbsent(
            ClassLoader classLoader,
            Function<? super ClassLoader, ? extends T> mappingFunction);

    /**
     * Removes the value associated with this ClassLoaderValue.
     * 
     * @param classLoader ClassLoader the value is associated with
     * @return old value assocaited with the ClassLoader
     */
    public abstract T remove(ClassLoader classLoader);

    /**
     * Removes the value associated with this ClassLoaderValue.
     * 
     * @param classLoader ClassLoader the value is associated with
     * @param value       value to remove
     * @return true if removed false otherwise
     */
    public abstract boolean remove(ClassLoader classLoader, Object value);

    /** Default constructor. */
    protected ClassLoaderValue() {
    }

    /**
     * Returns the underlying map for storing values for the ClassLoader.
     * 
     * @param classLoader ClassLoader to get or create the map for.
     * @return class loader value map
     */
    @SuppressWarnings("unchecked")
    protected static ConcurrentHashMap<Object, Object> map(ClassLoader classLoader) {
        return (ConcurrentHashMap<Object, Object>) (classLoader != null
                ? classLoader.createOrGetClassLoaderValueMap()
                : BootLoader.getClassLoaderValueMap());
    }
}
