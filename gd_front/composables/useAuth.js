import { useFetch, useCookie, useRuntimeConfig } from '#app'
export function useAuth() {
    const config = useRuntimeConfig()
    const authToken = useCookie('auth_token').value
    const user = useState('auth-user', () => null); // A global state for the user
    const router = useRouter();

    // Fetch current user
    async function fetchCurrentUser() {
        try {
            const { data, error } = await useFetch('/api/me', {
                method: 'GET',
                baseURL: config.public.apiBaseUrl,
                headers: {
                    Accept: `application/json`,
                    Authorization: `Bearer ${authToken}`
                }
            });
            if (error.value) {
                console.error('Failed to fetch user:', error.value)
                localStorage.setItem('user', null)
                await router.push('/login');
                return null
            } else {
                user.value = data.value; // Store user in global state
                localStorage.setItem('user', JSON.stringify(data.value.data))
                return user.value;
            }

        } catch (error) {
            if (error.response?.status === 401) {
                user.value = null; // Clear user if unauthorized
                localStorage.setItem('user', null)
                await router.push('/login'); // Redirect to login
            }
            throw error;
        }
    }

    return { user, fetchCurrentUser };
}
