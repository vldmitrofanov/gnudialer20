// middleware/authAdmin.js
// import { useCookie } from '#app'
import { navigateTo } from 'nuxt/app';


export default function ({ redirect }) {
    // const authToken = useCookie('auth_token').value;

    // Assume you decode the authToken to get user information
    const user = decodeToken(); // Replace with actual token decoding logic

    // If user id is not 1, redirect to unauthorized
    if (!user) {
        return navigateTo('/login');
    } else if (user.id != 1) {
        return navigateTo('/crm')
    }
}

function decodeToken() {
    // Implement your token decoding logic here
    // For example, if you're using JWT, you might use jwt-decode package
    return JSON.parse(localStorage.getItem('user'))
}