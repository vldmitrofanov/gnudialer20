<template>
    <div class="login-container">
      <a-card title="Login" class="login-card">
        <a-form @submit="handleSubmit" layout="vertical">
          <a-form-item label="Email">
            <a-input v-model:value="email" type="email" placeholder="Enter your email" required />
          </a-form-item>
          <a-form-item label="Password">
            <a-input v-model:value="password" type="password" placeholder="Enter your password" required />
          </a-form-item>
          <a-form-item>
            <a-button type="primary" html-type="submit" block>Login</a-button>
          </a-form-item>
        </a-form>
      </a-card>
    </div>
  </template>
  
  <script setup>
  import { useRouter } from 'vue-router'
  import { useCookie, useFetch } from '#app'
  
  const router = useRouter()
  
  const email = ref('')
  const password = ref('')
  
  const handleSubmit = async (event) => {
    event.preventDefault()
    try {
      // Step 1: Send login request to Laravel backend
      const { data, error } = await useFetch('/api/login', {
        method: 'POST',
        baseURL: useRuntimeConfig().public.apiBaseUrl,
        body: {
          email: email.value,
          password: password.value
        }
      })
  
      if (error.value) {
        throw new Error('Login failed')
      }
  
      // Step 2: Store the received token in cookies
      const authToken = data.value.token // Adjust according to your API response
      const cookie = useCookie('auth_token')
      cookie.value = authToken
      const fetchUser = await getMeUser(authToken)
      if(!fetchUser){
        alert('Login failed: Failed to fetch user')
        return
      }
      // Step 3: Redirect to the dashboard or home page
      router.push('/crm')
    } catch (err) {
      console.error(err)
      // Handle error, e.g., show a message to the user
      alert('Login failed: Invalid email or password')
      console.log(err)
    }
  }

  const getMeUser = async(authToken) => {
    const { data, error } = await useFetch('/api/me', {
    baseURL: useRuntimeConfig().public.apiBaseUrl,
    headers: {
        Authorization: `Bearer ${authToken}`
    }
    })

    if (error.value) {
        console.error('Failed to fetch user:', error.value)     
        return false
    } else {
        localStorage.setItem('user', JSON.stringify(data.value))
        return true
    }
}
  </script>
  
  <style scoped>
  .login-container {
    display: flex;
    justify-content: center;
    align-items: center;
    height: 100vh;
    background-color: #efefef;
  }
  
  .login-card {
    width: 380px;
  }
  </style>
  